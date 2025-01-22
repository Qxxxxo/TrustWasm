/**
 * @brief pta i2c service
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#ifndef __PTA_IO_HELPER_H
#define __PTA_IO_HELPER_H

typedef enum{
    PERIPH_GPIO=0,
    PERIPH_I2C,
    PERIPH_SPI,
    PERIPH_SECURE_STORAGE,
}periph_type_t;

typedef enum{
    DHT_READ,
    BH1750FVI_SAMPLE,
    I2C_READ_BYTES,
    I2C_WRITE_BYTES,
    I2C_READ_REGS,
    I2C_WRITE_REGS,
    SPI_TRANSFER_BYTES,
    SPI_TRANSFER_BYTE,
    SPI_TRANSFER_DIFF_BYTES,
    WRITE_SECURE_OBJECT,
    READ_SECURE_OBJECT,
    MPU6050_SAMPLE,
    TEL0157_GET_GNSS_LEN,
    TEL0157_GET_ALL_GNSS,
    TEL0157_GET_UTC_TIME,
    TEL0157_GET_GNSS_MODE,
    TEL0157_GET_NUM_SAT_USED,
    TEL0157_GET_LON,
    TEL0157_GET_LAT,
    TEL0157_GET_ALT,
    TEL0157_GET_SOG,
    TEL0157_GET_COG,
    AT24CXX_WRITE,
    AT24CXX_READ,
    ATK301_GET_FINGERPRINT_IMAGE,
    ATK301_UPLOAD_FINGERPRINT_IMAGE,
    BME280_SAMPLE,
    BME280_GET_CALIBRATION,
    REQ_TYPE_NONE,
}io_req_type_t;

// a84e4d9d-ad15-4cbc-83c9-d8e4a73935b5
#define PTA_IO_HELPER_UUID                                 \
    {                                                      \
        0xa84e4d9d, 0xad15, 0x4cbc,                        \
        {                                                  \
            0x83, 0xc9, 0xd8, 0xe4, 0xa7, 0x39, 0x35, 0xb5 \
        }                                                  \
    }                                                      

#define PTA_IO_HELPER_CMD_START_LOOP        0
#define PTA_IO_HELPER_CMD_POLL              1
#define PTA_IO_HELPER_CMD_READ_REQ          2
#define PTA_IO_HELPER_CMD_GET_DATA          3
#define PTA_IO_HELPER_CMD_POLL_SUBSIGNAL    4
#define PTA_IO_HELPER_CMD_WRITE_REQ         5
#define PTA_IO_HELPER_CMD_READ_REQ_WITH_BUFFER 6
#define PTA_IO_HELPER_CMD_WRITE_REQ_WITH_BUFFER 7
#define PTA_IO_HELPER_CMD_GET_OVERHEAD_TIMESTAMP 8

#endif

