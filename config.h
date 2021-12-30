/*
 * config.h
 *
 *  Created on: Mar 15, 2021
 *      Author: ubuntu
 */

#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_

#define TRUE		1
#define FALSE		0

#define ENABLE		1
#define DISABLE		0

#define MODE_STR_NOGPSDATA 0
#define MODE_STR_NUM 4
static char *mode_str[MODE_STR_NUM+1] = {"NO GPS DATA", "NO FIX", "2D FIX", "3D FIX", "UNK"};
static char *time_src_str[] = {"n/a", "System", "GPS"};

#define DEFAULT_GPSD_HOST "localhost"
#define NPPS_DEFAULT	4

#define LOG_INFO	0
#define LOG_WARN	1
#define LOG_ERROR	2
#define LOG_DEBUG1	11
#define LOG_DEBUG2	12

#define TIME_SRC_OS		1u
#define TIME_SRC_GPS	2u

#define SYS	TIME_SRC_OS
#define GPS	TIME_SRC_GPS


// PL UTC TIMESTAMP RTL MODULE, REGISTERS, MASKS
#define UTC_TIMESTAMP_MODULE_BASEADDR		0x80068000
#define UTC_SEC_REG_ADDR					0x00		// [W] bit<0-31>: 	UTC seconds
#define UTC_SRC_FS_REG_ADDR					0x04		// [W] bit<0-19>: 	fractional sec (for ms use bit<0-9> or for us use bit<0-19>)
														//     bit<20-23>:	time source: 0 - reset value, 1 - system, 2 - GPS
#define UTC_CONFIG_REG_ADDR					0x08		// [W] bit<0-3>:  	npps
#define UTC_PICOM_DATA_LSB_REG_ADDR			0x0C		// [R] bit<0-31>:	PiCom UTC timestamp bit<0-31> (data send to PiCom module)
#define UTC_PICOM_DATA_MSB_REG_ADDR			0x10		// [R] bit<0-23>:	PiCom UTC timestamp bit<32-55> (data send to PiCom module)

#define REG_GPS_SRC_MSK 0x00200000
#define REG_SYS_SRC_MSK 0x00100000

// APU-RPU SHARED MEMORY REGISTERS
#define APU_RPU_SHM_STATUSWATCH_BASEADDR	0x3000FF00
#define STW_PICOM_GPS_CONFIG_REG_ADDR		0xA0		// statuswatch register 40 | bit<0-3>:	npps
#define STW_PICOM_UTC_MSB_REG_ADDR			0xA4		// statuswatch register 41 |
#define STW_PICOM_UTC_LSB_REG_ADDR			0xA8		// statuswatch register 42 |

// APP CONFIG
#define APP_USE_APU_RPU_SHM
#define APP_USE_MS

// global variables
char *appname;
unsigned int mode_dbg=0;

#endif /* SRC_CONFIG_H_ */
