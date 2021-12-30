/*
 * gpsd client gps2pl.c
 *
 *  Created on: Mar 14, 2021
 *      Author: Kostya A.
 *
 * Description: app send gps time to PL
 *
 */

#include <gps.h>
#include <math.h>       // for isfinite()
#include <unistd.h>     // for sleep()
#include <string.h>		// for strcmp()
#include <stdlib.h>		// for calloc()
#include <stdarg.h>		// for va_start()
#include <fcntl.h>

#include "config.h"
#include "revision.h"
#include "mem_rw.h"

void print_help(char* appname)
{
	printf("%s version %s.%s (%s)\n\n\r", appname, VERSION, REVISION, __DATE__);
	printf("Description: gpsd service client sends UTC time to PL \n\n\r");
	printf("Usage: %s [-h] [-D n] \n\r", appname);
	printf("\t-D n\tset debug mode level n=<1-3>:  \n\r");
	printf("\t    \t1 = client data, 2 = timestamps data, 3 = all data \n\r");
	printf("\t-h\tShow this help, then exit\n\r");
}

void app_log(const int level, char* format, ...)
{
	char *lvl_str;
	char buffer[256];

	switch (mode_dbg) {
		case 0:
			if(level==LOG_DEBUG1 || level==LOG_DEBUG2)
				return;
			break;
		case 1:
			if(level==LOG_DEBUG2)
				return;
			break;
		case 2:
			if(level==LOG_DEBUG1)
				return;
			break;
	}

	switch (level) {
		case LOG_INFO:
			lvl_str = "INFO: ";
			break;
		case LOG_WARN:
			lvl_str = "WARNING: ";
			break;
		case LOG_ERROR:
			lvl_str = "ERROR: ";
			break;
		case LOG_DEBUG1:
			lvl_str = "DEBUG1: ";
			break;
		case LOG_DEBUG2:
			lvl_str = "DEBUG2: ";
			break;
		default:
			lvl_str = "UNK: ";
		}

	va_list args;
	va_start(args, format);
	vsprintf(buffer, format, args);
	printf("%s:%s%s", appname, lvl_str, buffer);
	va_end(args);
}

int getpidof(char* appname)
{
	#define STRLEN 50
	int npid=0;
	char *token;
    FILE *fp;
    char str1[STRLEN]={0}, str2[STRLEN]={0}, str3[STRLEN]={0};	// instead of: char *str1=(char*)calloc(1,50);
    sprintf(str1,"pidof %s",appname);
    fp=popen(str1,"r");
    fread(str2,1,500,fp);
    fclose(fp);
    strcpy(str3, str2);
    token = strtok(str3, " ");
    while(token != NULL) {
    	npid++;
    	token = strtok(NULL, " ");
    }
    app_log(LOG_INFO, "PID (by name): %s", str2);
    return npid;
}


int open_gpd_port(struct gps_data_t *gps_data)
{
	app_log(LOG_INFO, "opening gpsd %s port %s\n", DEFAULT_GPSD_HOST, DEFAULT_GPSD_PORT);
	if (0 != gps_open(DEFAULT_GPSD_HOST, DEFAULT_GPSD_PORT, gps_data)) {
		app_log(LOG_ERROR, "can't open gpsd %s port %s\n", DEFAULT_GPSD_HOST, DEFAULT_GPSD_PORT);
	   	app_log(LOG_ERROR, "maybe gpsd is not running!\n");
	   	sleep(1);
	   	return FALSE;
	} else {
		app_log(LOG_INFO, "start listening to gpsd %s port %s\n", DEFAULT_GPSD_HOST, DEFAULT_GPSD_PORT);
		return TRUE;
	}
}

int recovery_gps_strem(struct gps_data_t *gps_data)
{
    unsigned int done=FALSE;

    app_log(LOG_INFO, "start recovery of gps stream\n");
    app_log(LOG_INFO, "close current gps stream\n");
    (void)gps_stream(gps_data, WATCH_DISABLE, NULL);
    (void)gps_close(gps_data);
    sleep(3);
    do {
    	done = open_gpd_port(gps_data);
    } while (done==FALSE);

    // ask gpsd to stream reports
    (void)gps_stream(gps_data, WATCH_ENABLE | WATCH_JSON, NULL);

    app_log(LOG_INFO, "end recovery of gps stream\n");

    return 1;
}

int set_npps(void)
{
	unsigned int npps = NPPS_DEFAULT;
	int reg=0;
	int reg_addr=APU_RPU_SHM_STATUSWATCH_BASEADDR + STW_PICOM_GPS_CONFIG_REG_ADDR;

#ifdef APP_USE_APU_RPU_SHM
	mem_read(reg_addr, &reg);
	if(reg > 0)
		npps = reg & 0xF;
#endif

	app_log(LOG_INFO, "APU-RPU SHM STW_GPS_CONFIG register 0x%X value: 0x%X \n", reg_addr, reg);
    app_log(LOG_INFO, "client npps value: %d %s\n", npps, reg ? "(STW_GPS_CONFIG bit<0-3>)" : "(default value)");

	return npps;
}

int gpio_init(void)
{
	int exportfd, directionfd, valuefd;

	valuefd = open("/sys/class/gpio/gpio485/value", O_RDWR);
	if (valuefd > -1) {
		close(valuefd);
		return 0;
	}

	exportfd = open("/sys/class/gpio/export", O_WRONLY);
	if (exportfd < 0)	{
		printf("%s @ line %d: error opening /sys/class/gpio/export \n", __func__, __LINE__);
		return 1;
	}

	if(write(exportfd, "485", 4) < 1)	{
		printf("%s @ line %d: error writing to /sys/class/gpio/export \n", __func__, __LINE__);
		close(exportfd);
		return 2;
	}

	close(exportfd);

	directionfd = open("/sys/class/gpio/gpio485/direction", O_RDWR);
	if (directionfd < 0)	{
		printf("%s @ line %d: error opening /sys/class/gpio/gpio485/direction \n", __func__, __LINE__);
		return 3;
	}

	if(write(directionfd, "out", 4) < 1)	{
		printf("%s @ line %d: error writing to /sys/class/gpio/gpio485/direction \n", __func__, __LINE__);
		close(directionfd);
		return 4;
	}

	close(directionfd);

	return 0;
}
/*
 * 	toggle axi gpio signal connected to J14 connector
 *
 * 	@input:		none
 * 	@return:	none
 */
unsigned int gpio_value=0;

int gpio_handler(void)
{
	int valuefd;

	valuefd = open("/sys/class/gpio/gpio485/value", O_RDWR);
	if (valuefd < 0)	{
		printf("%s @ line %d: error opening /sys/class/gpio/gpio485/value \n", __func__, __LINE__);
		return 1;
	}

	if(gpio_value == 0) {
		if(write(valuefd, "0", 2) < 1)	{
			printf("%s @ line %d: error writing value 0 to /sys/class/gpio/gpio485/value \n", __func__, __LINE__);
			close(valuefd);
			return 2;
		}
		gpio_value = 1;
	}
	else if (gpio_value == 1) {
		if(write(valuefd, "1", 2) < 1)	{
			printf("%s @ line %d: error writing value 1 to /sys/class/gpio/gpio485/value \n", __func__, __LINE__);
			close(valuefd);
			return 3;
		}
		gpio_value = 0;
	}

	close(valuefd);

	return 0;
}

/*
 * 	main function
 */
int main(int argc, char *argv[])
{
    struct gps_data_t gps_data;
    timestamp_t time, prev_time;
    unsigned int time_s, time_ms;
    unsigned int src_flg=GPS, data_ready_flg=FALSE, time_src=0;
    unsigned int npps, done=FALSE;
    unsigned int gps_waiting_timeout_us, use_system_time_flg_max, use_system_time_flg_cntr=0;
    unsigned int gps_read_err_cntr=0, gps_read_err_max;
    unsigned int i, help_flg=FALSE;
    int reg, reg1, reg2, reg3, reg4, gpio_init_flg=0;
    char isotime[128];
    unsigned long utc_ts=0, picom_ts=0;
    double utc_ts_sec;

    // init variables
    appname = argv[0];

    // parsing arguments
    for(i=1; i<argc; i++)
    {
    	if ( strcmp(argv[i],"-h") == 0 ) {
    		help_flg = TRUE;
    		continue;
    	}
    	if ( strcmp(argv[i],"-D") == 0 ) {
    		if(argc > i+1){
    			mode_dbg = atoi(argv[i+1]);
    			if(mode_dbg<1 || mode_dbg>3) {
    			    printf("Incorrect Debug mode level \n\r");
    			    return 0;
    			    }
    		}
    		else
    		{
    			printf("Debug mode level undefined \n\r");
    			return 0;
    		}
    	    continue;
    	}
    }

    if(help_flg) {
    	print_help(appname);
    	return 0;
    }

    // print launching app message
    app_log(LOG_INFO, "launching %s version %s.%s (%s)\n", appname, VERSION, REVISION, __DATE__);

    // check if gps2pl process is already running
    if( getpidof(appname) > 1) {
    	app_log(LOG_ERROR, "client is running already \n");
    	app_log(LOG_INFO, "exit client \n");
    	return 1;
    }

    // print debug mode status message
    app_log(LOG_INFO, "debug mode status: %s\n", mode_dbg ? "enable" : "disable");
    // init gpio
    if(gpio_init() == 0) {
    	gpio_init_flg = 1;
    }
    app_log(LOG_INFO, "gpio init status: %s\n", gpio_init_flg ? "ok" : "fail");


    // init global variable prev_time
    prev_time = 1609459200.000; // January 1, 2021 0:00:00, UTC
    unix_to_iso8601(prev_time, isotime, sizeof(isotime));
    app_log(LOG_INFO, "init previous time value: prev_time = %.3f (UTC %s)\n", (double)prev_time, isotime);


	app_log(LOG_INFO, "PL UTC timestamp module Base Address: 0x%X \n", UTC_TIMESTAMP_MODULE_BASEADDR);
	app_log(LOG_INFO, "PL UTC timestamp module Reg#0: [W], addr 0x%02X, bit[0-31] = UTC_SEC \n", UTC_SEC_REG_ADDR);
	app_log(LOG_INFO, "PL UTC timestamp module Reg#1: [W], addr 0x%02X, bit[0-9],[20-23] = UTC_mSEC, UTC_source \n", UTC_SRC_FS_REG_ADDR);
	app_log(LOG_INFO, "PL UTC timestamp module Reg#2: [W], addr 0x%02X, bit[0-3] = GPS_NPPS \n", UTC_CONFIG_REG_ADDR);
	app_log(LOG_INFO, "PL UTC timestamp module Reg#3: [R], addr 0x%02X, bit[0-31] = PiCom Timestamp bit[0-31] \n", UTC_PICOM_DATA_LSB_REG_ADDR);
	app_log(LOG_INFO, "PL UTC timestamp module Reg#4: [R], addr 0x%02X, bit[0-23] = PiCom Timestamp bit[32-55] \n", UTC_PICOM_DATA_MSB_REG_ADDR);

	 // init global variable npps
	npps = set_npps();
	reg2 = npps;
	mem_write(UTC_TIMESTAMP_MODULE_BASEADDR+UTC_CONFIG_REG_ADDR, reg2);
	mem_read(UTC_TIMESTAMP_MODULE_BASEADDR+UTC_CONFIG_REG_ADDR, &reg);
	if(reg2 == reg)
		app_log(LOG_INFO, "set PL UTC timestamp module Reg#2 value: 0x%X (verified)\n", reg2);
	else
		app_log(LOG_ERROR, "set PL UTC timestamp module Reg#2 value: 0x%X (verification fail !!!) \n", reg2);


    // opening gpsd port
    do {
    	done = open_gpd_port(&gps_data);
    } while (done==FALSE);

    // ask gpsd to stream reports
    (void)gps_stream(&gps_data, WATCH_ENABLE | WATCH_JSON, NULL);

    // update main loop variables
    gps_waiting_timeout_us = 1000000 / npps;
    gps_waiting_timeout_us = gps_waiting_timeout_us * 1.5;
    use_system_time_flg_max = npps;
    gps_read_err_max = use_system_time_flg_max * 5;

    // main loop
    while (1) {
    	// use gps data first
    	if(src_flg == GPS)
		{
    		// wait for gps data
			if(gps_waiting(&gps_data, gps_waiting_timeout_us))
			{
				// read from a gpsd connection
				if (-1 == gps_read(&gps_data)) {
					app_log(LOG_ERROR, "read from a gpsd connection failed: gps_read() returns -1\n");
					src_flg = SYS;
					gps_read_err_cntr++;
					if(gps_read_err_cntr == gps_read_err_max) {
						recovery_gps_strem(&gps_data);
						gps_read_err_cntr = 0;
					}
					continue;
				} else {
					gps_read_err_cntr = 0;
				}
				if (MODE_SET != (MODE_SET & gps_data.set)) {
					app_log(LOG_DEBUG1, "did not get mode from gpsd data (MODE_SET != 1)\n");
					src_flg = SYS;
					continue;
				}
				if (0 > gps_data.fix.mode || MODE_STR_NUM <= gps_data.fix.mode) {
					app_log(LOG_DEBUG1, "Unknown GPS Status\n");
					gps_data.fix.mode = MODE_STR_NUM;
					src_flg = SYS;
					continue;
				}
				if (TIME_SET == (TIME_SET & gps_data.set)) {
					time = gps_data.fix.time;
					if(time > prev_time) {
						time_s = (unsigned int)time;
						time_ms = (time - time_s)*1000;
						reg1 = REG_GPS_SRC_MSK | time_ms;
						time_src = TIME_SRC_GPS;
						data_ready_flg = TRUE;
						use_system_time_flg_cntr = 0;
						prev_time = time;
					}
					else {
						app_log(LOG_DEBUG1, "GPS Status: %s, UTC incorrect value %.3f while previous time value %.3f \n",
								mode_str[gps_data.fix.mode], (double)time, (double)prev_time);
						src_flg = SYS;
					}
				} else {
					app_log(LOG_DEBUG1, "GPS Status: %s, UTC Time: not available (TIME_SET != 1) \n", mode_str[gps_data.fix.mode]);
					src_flg = SYS;
				}
			} else {
				app_log(LOG_DEBUG1, "GPS waiting timeout\n");
				src_flg = SYS;
			}
		}

    	// no gps data handler
    	if(src_flg == SYS) {
    		gps_data.fix.mode = MODE_STR_NOGPSDATA;
    		use_system_time_flg_cntr++;

			if(use_system_time_flg_cntr >= use_system_time_flg_max) {
				use_system_time_flg_cntr = 0;
				time = timestamp();
				time_s = (unsigned int)time;
				time_ms = (time - time_s)*1000;
				reg1 = REG_SYS_SRC_MSK | time_ms;
				time_src = TIME_SRC_OS;
				data_ready_flg = TRUE;
			}
			src_flg = GPS;
    	}

    	// send time to PL module handle
    	if (data_ready_flg && time_src)
    	{
    		// send date to PL module
    		mem_write(UTC_TIMESTAMP_MODULE_BASEADDR+UTC_SEC_REG_ADDR, time_s);
    		mem_write(UTC_TIMESTAMP_MODULE_BASEADDR+UTC_SRC_FS_REG_ADDR, reg1);

    		// print debug info
    		app_log(LOG_DEBUG1, "Source: %s (%s), UTC: %.3f => PL UTC: Reg#0: %d [0x%08X], Reg#1: %d | %3d [0x%08X]\n",
    				time_src_str[time_src], mode_str[gps_data.fix.mode], (double)time, time_s, time_s, time_src, time_ms, reg1);

    		data_ready_flg = FALSE;
    		time_src = 0;

    		// print timestamps and offset
    		if(mode_dbg==2 || mode_dbg==3) {
//    			utc_ts = mem_reg32b_read(UTC_TIMESTAMP_MODULE_BASEADDR+UTC_PICOM_DATA_MSB_REG_ADDR);
//    			utc_ts = (utc_ts << 32) | mem_reg32b_read(UTC_TIMESTAMP_MODULE_BASEADDR+UTC_PICOM_DATA_LSB_REG_ADDR);
//        		app_log(LOG_DEBUG2, "Timestamps: PL UTC = %d => PiCom UTC = %d; Offset = %d us \n", utc_ts, picom_ts, (picom_ts-utc_ts));
    			mem_read(UTC_TIMESTAMP_MODULE_BASEADDR+UTC_PICOM_DATA_LSB_REG_ADDR, &reg3);
    			mem_read(UTC_TIMESTAMP_MODULE_BASEADDR+UTC_PICOM_DATA_MSB_REG_ADDR, &reg4);
    			utc_ts = (unsigned long)reg4;
    			utc_ts = utc_ts << 32;
    			utc_ts = utc_ts | (unsigned int)reg3;
    			utc_ts_sec = (double)utc_ts/1000000.0;
    			app_log(LOG_DEBUG2, "Timestamps: PL UTC: %.6f (Reg#4=0x%08X, Reg#3=0x%08X => 0x%llx / %lu) \n", utc_ts_sec, reg4, reg3, utc_ts, utc_ts);

    		}

    		// toggle gpio
    		if (gpio_init_flg)
    			gpio_handler();
    	}

    }

    (void)gps_stream(&gps_data, WATCH_DISABLE, NULL);
    (void)gps_close(&gps_data);
    return 0;
}



