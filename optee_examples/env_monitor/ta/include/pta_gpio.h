/**
 * @brief pta gpio service
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#ifndef __PTA_GPIO_H
#define __PTA_GPIO_H


#define PTA_GPIO_SERVICE_UUID \
		{ 0xa2e8677d, 0x4f37, 0x4cb8, \
			{ 0xa1, 0xb9, 0xdf, 0x52, 0x32, 0x40, 0x45, 0x98 } }


/*
 * GPIO INIT
 * [in] value[0].a: gpio pin number
 * [in] value[0].b: gpio mode 
 */
#define PTA_GPIO_CMD_INIT  0

/*
 * GPIO SET
 * [in] value[0].a: gpio pin number
 * [in] value[0].b: value set to pin
 */
#define PTA_GPIO_CMD_SET   1

/*
 * GPIO GET
 * [in] value[0].a: gpio pin number
 * [out] value[1].a: value get from pin
 */
#define PTA_GPIO_CMD_GET   2

#define PTA_GPIO_CMD_TEST_OVERHEAD 3

#endif