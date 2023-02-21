/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various existing       */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "..\core\fatfs\source\ff.h"        /* Obtains integer types */
#include "..\core\fatfs\source\diskio.h"    /* Declarations of disk functions */

#include <stdio.h>		// printf() for debugging
#include <string.h>     // memcpy()
#include "sd_spi.h"


DWORD get_fattime (void) {
	rtccTime tm;	// time structure
	rtccDate dt;	// date structure
	RtccGetTimeDate(&tm, &dt);
	unsigned char y=((dt.year>>4) & 15)*10+(dt.year & 15);
	if(y>99) y=0;
	unsigned char m=((dt.mon>>4) & 15)*10+(dt.mon & 15);
	if(m<1 || m>12) m=1;
	unsigned char d=((dt.mday>>4) & 15)*10+(dt.mday & 15);
	if(d<1 || d>31) d=1;
	unsigned char h=((tm.hour>>4) & 15)*10+(tm.hour & 15);
	if(h>23) h=0;
	unsigned char n=((tm.min>>4) & 15)*10+(tm.min & 15);
	if(n>59) n=0;
	unsigned char s=((tm.sec>>4) & 15)*10+(tm.sec & 15);
	if(s>59) s=0;
    return ((DWORD)((2000+y) - 1980) << 25)	// Year
         | ((DWORD)m << 21)					// Month
         | ((DWORD)d << 16)					// Day
         | ((DWORD)h << 11)					// Hour
         | ((DWORD)n << 5)					// Minute
         | ((DWORD)s >> 1);					// Second
}


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive number to identify the drive */
) {
	DSTATUS stat=0;

	switch (pdrv) {
        case DEV_NUL:
            return stat;

        case DEV_IFS:
            // there is nothing needed here
            return stat;

        case DEV_RAM:
            if(!ramdisk) stat|=STA_NODISK;
            return stat;

        case DEV_SD1:
			currentSD=0;
			stat=sd_status(0);
            return stat;

		case DEV_SD2:
			currentSD=1;
            stat=sd_status(0);
            return stat;

	}
	return STA_NOINIT;
}


/*-----------------------------------------------------------------------*/
/* Initialise a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive number to identify the drive */
) {
	DSTATUS stat=0;

	switch (pdrv) {
        case DEV_NUL:
            return stat;

        case DEV_IFS:
            // there is nothing needed here
            return stat;

        case DEV_RAM:
            if(!ramdisk) {
                xalloc((unsigned char **)&ramdisk,(1024*RAM_DISK_SIZE));    // allocate RAM disk memory
                if(!ramdisk) stat|=STA_NODISK;
            }
            return stat;

        case DEV_SD1:
			currentSD=0;
			init_sys_SPI(3,SD_CARD_CLK);
			stat=sd_init(0);
            return stat;

		case DEV_SD2:
			currentSD=1;
			init_sys_SPI(3,SD_CARD_CLK);
            stat=sd_init(0);
            return stat;
	}
	return STA_NOINIT;
}


/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive number to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
) {
	DRESULT res=FR_OK;

	switch (pdrv) {
        case DEV_NUL:
            return res;

        case DEV_IFS:
            if(sector<((IFS_SIZE*1024)/BYTE_ROW_SIZE)) memcpy((unsigned char *)buff,(unsigned char *)(ifs+(sector*BYTE_ROW_SIZE)),(count*BYTE_ROW_SIZE));
            else res=RES_PARERR;
            return res;

        case DEV_RAM:
            if(ramdisk) {
                if(sector<((RAM_DISK_SIZE*1024)/FatFs.ssize)) {
                    memcpy((unsigned char *)buff,(unsigned char *)(ramdisk+(sector*FatFs.ssize)),(count*FatFs.ssize));
                }
                else res=RES_PARERR;
            } else res=RES_NOTRDY;
            return res;

        case DEV_SD1:
			currentSD=0;
			res=sd_read(0,buff,sector,count);
            return res;

		case DEV_SD2:
			currentSD=1;
            res=sd_read(0,buff,sector,count);
            return res;
	}

	return RES_PARERR;
}


/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive number to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
) {
	DRESULT res=FR_OK;

	switch (pdrv) {
        case DEV_NUL:
            return res;

        case DEV_IFS:
            if(sector<((IFS_SIZE*1024)/BYTE_ROW_SIZE)) {
                unsigned char *pagebuff=NULL;
                xalloc((unsigned char **)&pagebuff,BYTE_PAGE_SIZE);
                if(pagebuff) {
                    int ss=
                    #if FF_MIN_SS!=FF_MAX_SS
                        FatFs.ssize;
                    #else
                        FF_MAX_SS;
                    #endif
                    NVMProgram((unsigned char *)(ifs+(sector*ss)), (unsigned char *)buff, (count*ss), pagebuff);
                    xfree((unsigned char **)&pagebuff);
                } else res=RES_ERROR;
            }
            else res=RES_PARERR;
            return res;

        case DEV_RAM:
            if(ramdisk) {
                if(sector<((RAM_DISK_SIZE*1024)/FatFs.ssize)) {
                    memcpy((unsigned char *)(ramdisk+(sector*FatFs.ssize)),(unsigned char *)buff,(count*FatFs.ssize));
                }
                else res=RES_PARERR;
            } else res=RES_NOTRDY;
            return res;

        case DEV_SD1:
			currentSD=0;
			res=sd_write(0,buff,sector,count);
            return res;

		case DEV_SD2:
			currentSD=1;
            res=sd_write(0,buff,sector,count);
            return res;
	}

	return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive number (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
) {
	DRESULT res=FR_OK;
    WORD h;
    DWORD v;

	switch (pdrv) {
        case DEV_NUL:
            return res;

        case DEV_IFS:
            switch(cmd) {
                case GET_SECTOR_COUNT:
                    v=(IFS_SIZE*1024)/BYTE_ROW_SIZE;
                    memcpy((unsigned char *)buff,(unsigned char *)&v,sizeof(DWORD));
                    break;

                case GET_SECTOR_SIZE:
                    h=BYTE_ROW_SIZE;
                    memcpy((unsigned char *)buff,(unsigned char *)&h,sizeof(WORD));
                    break;

                case GET_BLOCK_SIZE:
                    v=NUM_ROWS_PAGE;
                    memcpy((unsigned char *)buff,(unsigned char *)&v,sizeof(DWORD));
                    break;

            }
            return res;

        case DEV_RAM:
            switch(cmd) {
                case GET_SECTOR_COUNT:
                    v=(RAM_DISK_SIZE*1024)/512;
                    memcpy((unsigned char *)buff,(unsigned char *)&v,sizeof(DWORD));
                    break;

                case GET_SECTOR_SIZE:
                    h=512;
                    memcpy((unsigned char *)buff,(unsigned char *)&h,sizeof(WORD));
                    break;

                case GET_BLOCK_SIZE:
                    v=1;    // not flash memory
                    memcpy((unsigned char *)buff,(unsigned char *)&v,sizeof(DWORD));
                    break;

            }
            return res;

        case DEV_SD1:
			currentSD=0;
			res=sd_ioctl(0,cmd,buff);
            return res;

		case DEV_SD2:
			currentSD=1;
            res=sd_ioctl(0,cmd,buff);
            return res;

    }

	return RES_PARERR;
}
