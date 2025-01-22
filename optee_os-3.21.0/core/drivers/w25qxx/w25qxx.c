/**
 * @brief w25qxx driver
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */

#include <drivers/w25qxx/w25qxx.h>
#include <kernel/tee_time.h>
#include <trace.h>

#define WRITE_ENABLE_TIMEOUT_US    2000
#define SECTOR_ERASE_TIMEOUT_US    2000000
#define CHIP_ERASE_TIMEOUT_US      80000000
#define PAGE_PROGRAM_TIMEOUT_PER_BYTE_US   10000

static inline uint64_t teetime_micro(TEE_Time *t)
{
    return t->seconds * 1000000 + t->micros;
}

static TEE_Result w25qxx_spi_write(spi_t bus, spi_cs_t cs, uint8_t cmd, const void * data, size_t len){
    // TEE_Result res;
    // res=spi_transfer_byte(bus, cs, false,cmd,NULL); // send cmd
    // if(res!=TEE_SUCCESS){
    //     return res;
    // }
    // if(data&&len>0){
    //     return spi_transfer_bytes(bus,cs,true,data,NULL,len); // send data
    // }
    // return TEE_SUCCESS;
    return spi_transfer_regs(bus,cs,cmd,data,NULL,len); // send data
}

static TEE_Result w25qxx_spi_read(spi_t bus, spi_cs_t cs,uint8_t cmd, void *data, size_t len) {
    // TEE_Result res;
    // res=spi_transfer_byte(bus, cs, false,cmd,NULL);  // send cmd
    // if(res!=TEE_SUCCESS){
    //     return res;
    // }
    // if(data&&len>0){
    //     return spi_transfer_bytes(bus, cs,true, NULL, data, len);  // recv data
    // }
    // return TEE_SUCCESS;
    return spi_transfer_regs(bus,cs,cmd,NULL,data,len); // recv data
}

static inline TEE_Result w25qxx_read_status_internal(spi_t bus, spi_cs_t cs, uint8_t * status){
    return w25qxx_spi_read(bus,cs,W25QXX_CMD_READ_STATUS,status,1);
}

static inline TEE_Result w25qxx_wait_until_ready_internal(spi_t bus, spi_cs_t cs, uint32_t timeout){
    uint8_t status;
    TEE_Time start,end;
    TEE_Result res;
    tee_time_get_sys_time(&start);
    do{
        res=w25qxx_read_status_internal(bus,cs,&status);
        if(res!=TEE_SUCCESS){
            return res;
        }
        tee_time_get_sys_time(&end);
    }while((status & W25QXX_STATUS_BUSY)&&teetime_micro(&end)-teetime_micro(&start)<=timeout);
    if(teetime_micro(&end)-teetime_micro(&start)>timeout) return TEE_ERROR_TIMEOUT;
    return TEE_SUCCESS;
}

static inline TEE_Result w25qxx_write_enable_internal(spi_t bus, spi_cs_t cs){
    TEE_Result res;
    res=w25qxx_spi_write(bus,cs,W25QXX_CMD_WRITE_ENABLE,NULL,0);
    if(res!=TEE_SUCCESS){
        return res;
    }
    // DMSG("[w25qxx] write finish, wait ready");
    return w25qxx_wait_until_ready_internal(bus,cs,WRITE_ENABLE_TIMEOUT_US);
}

static inline TEE_Result w25qxx_write_disable_internal(spi_t bus, spi_cs_t cs){
    TEE_Result res;
    res=w25qxx_spi_write(bus,cs,W25QXX_CMD_WRITE_DISABLE,NULL,1);
    if(res!=TEE_SUCCESS){
        return res;
    }
    return w25qxx_wait_until_ready_internal(bus,cs,WRITE_ENABLE_TIMEOUT_US);
}

TEE_Result w25qxx_wait_until_ready(spi_t bus, spi_cs_t cs, uint32_t timeout){
    TEE_Result res;
    spi_acquire(bus,cs,SPI_MODE_0,SPI_CLK_40MHZ);
    res=w25qxx_wait_until_ready_internal(bus,cs,timeout);
    spi_release(bus);
    return res;
}

TEE_Result w25qxx_read_status(spi_t bus, spi_cs_t cs,uint8_t * status){
    TEE_Result res;
    spi_acquire(bus,cs,SPI_MODE_0,SPI_CLK_40MHZ);
    res=w25qxx_spi_read(bus,cs,W25QXX_CMD_READ_STATUS,status,1);
    spi_release(bus);
    return res;
}

TEE_Result w25qxx_write_status(spi_t bus, spi_cs_t cs, uint8_t status){
    TEE_Result res;
    spi_acquire(bus,cs,SPI_MODE_0,SPI_CLK_40MHZ);
    res=w25qxx_spi_write(bus,cs,W25QXX_CMD_WRITE_STATUS,&status,1);
    spi_release(bus);
    return res;
}

TEE_Result w25qxx_init(spi_t bus, spi_cs_t cs){
    TEE_Result res;
    DMSG("[w25qxx] try to acquire spi%d, cs%d",bus,cs);
    spi_acquire(bus,cs,SPI_MODE_0,SPI_CLK_40MHZ);
    DMSG("[w25qxx] write enable spi%d, cs%d",bus,cs);
    res=w25qxx_write_enable_internal(bus,cs); 
    DMSG("[w25qxx] write enable done");
    spi_release(bus);
    return res;
}

TEE_Result w25qxx_write_enable(spi_t bus, spi_cs_t cs){
    TEE_Result res;
    spi_acquire(bus,cs,SPI_MODE_0,SPI_CLK_40MHZ);
    res=w25qxx_write_enable_internal(bus,cs); 
    spi_release(bus);
    return res;
}

TEE_Result w25qxx_read_id(spi_t bus, spi_cs_t cs, uint16_t * id){
    TEE_Result res;
    uint8_t out[3];
    res=spi_acquire(bus,cs,SPI_MODE_0,SPI_CLK_40MHZ);
    if(res!=TEE_SUCCESS){
        return res;
    }
    res=spi_transfer_regs(bus,cs,W25QXX_CMD_READ_ID,NULL,out,3);
    // res=w25qxx_spi_read(bus,cs,W25QXX_CMD_READ_ID,out,3);
    if(res==TEE_SUCCESS){
        *id |= out[1] << 8;
        *id |= out[2];
    }
    spi_release(bus);
    return res;
}

TEE_Result w25qxx_write_disable(spi_t bus, spi_cs_t cs){
    TEE_Result res;
    spi_acquire(bus,cs,SPI_MODE_0,SPI_CLK_40MHZ);
    res=w25qxx_write_disable_internal(bus,cs);
    spi_release(bus);
    return res;
}

TEE_Result w25qxx_read_data(spi_t bus, spi_cs_t cs, uint8_t * buf, uint32_t read_addr, uint32_t len){
    TEE_Result res;
    uint8_t addr[3]={
        (read_addr>>16)&0xFF,
        (read_addr>>8)&0xFF,
        read_addr&0xFF
    };
    spi_acquire(bus,cs,SPI_MODE_0,SPI_CLK_40MHZ);
    res=w25qxx_spi_write(bus,cs,W25QXX_CMD_READ_DATA,addr,3); // send read data cmd & read address
    if(res!=TEE_SUCCESS){
        spi_release(bus);
        return res;
    }
    res=spi_transfer_bytes(bus,cs,false,NULL,buf,len); // read data
    spi_release(bus);
    return TEE_SUCCESS;
}

TEE_Result w25qxx_page_program(spi_t bus, spi_cs_t cs, const uint8_t * buf, uint32_t write_addr, uint32_t len){
    TEE_Result res;
    uint8_t addr[3]={
        (write_addr>>16)&0xFF,
        (write_addr>>8)&0xFF,
        write_addr&0xFF
    };
    res=spi_acquire(bus,cs,SPI_MODE_0,SPI_CLK_40MHZ);
    if(res!=TEE_SUCCESS){
        return res;
    }
    res=w25qxx_write_enable_internal(bus,cs); // enable write
    if(res!=TEE_SUCCESS){
        spi_release(bus);
        return res;
    }
    res=w25qxx_spi_write(bus,cs,W25QXX_CMD_PAGE_PROGRAM,addr,3);
    if(res!=TEE_SUCCESS){
        spi_release(bus);
        return res;
    }
    res=spi_transfer_bytes(bus,cs,true,buf,NULL,len);
    if(res!=TEE_SUCCESS){
        spi_release(bus);
        return res;
    }
    res=w25qxx_wait_until_ready_internal(bus,cs,PAGE_PROGRAM_TIMEOUT_PER_BYTE_US*len); // wait ready
    spi_release(bus);
    return TEE_SUCCESS;
}

// wait for ~150ms
TEE_Result w25qxx_sector_erase(spi_t bus, spi_cs_t cs, uint32_t erase_addr){
    TEE_Result res;
    uint8_t addr[3]={
        (erase_addr>>16)&0xFF,
        (erase_addr>>8)&0xFF,
        erase_addr & 0xFF
    };
    spi_acquire(bus,cs,SPI_MODE_0,SPI_CLK_40MHZ);
    res=w25qxx_write_enable_internal(bus,cs);
    if(res!=TEE_SUCCESS){
        spi_release(bus);
        return res;
    }
    res=w25qxx_spi_write(bus,cs,W25QXX_CMD_SECTOR_ERASE,addr,3);
    if(res!=TEE_SUCCESS){
        spi_release(bus);
        return res;
    }
    res=w25qxx_wait_until_ready_internal(bus,cs,SECTOR_ERASE_TIMEOUT_US);
    spi_release(bus);
    return TEE_SUCCESS;
}

// wait for a long time
TEE_Result w25qxx_chip_erase(spi_t bus, spi_cs_t cs){
    TEE_Result res;
    spi_acquire(bus,cs,SPI_MODE_0,SPI_CLK_40MHZ);
    res=w25qxx_write_enable_internal(bus,cs);
    if(res!=TEE_SUCCESS){
        spi_release(bus);
        return res;
    }
    res=w25qxx_spi_write(bus,cs,W25QXX_CMD_CHIP_ERASE,NULL,0);
    if(res!=TEE_SUCCESS){
        spi_release(bus);
        return res;
    }
    res=w25qxx_wait_until_ready_internal(bus,cs,CHIP_ERASE_TIMEOUT_US);
    spi_release(bus);
    return TEE_SUCCESS;
}