/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various existing       */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "..\core\fatfs\source\diskio.h"
#include "platform.h"   // declarations


DWORD get_fattime (void) {
	time_t tt;
	time(&tt);
	struct tm *lt=localtime(&tt);
	return ((DWORD)(lt->tm_year - 80) << 25)
         | ((DWORD)(lt->tm_mon+1) << 21)
         | ((DWORD)lt->tm_mday << 16)
         | ((DWORD)lt->tm_hour << 11)
         | ((DWORD)lt->tm_min << 5)
         | ((DWORD)lt->tm_sec >> 1);
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
			ifs=fopen("ifs.dat","rb");
			if(!ifs) stat|=STA_NODISK;
			fclose(ifs);
			return stat;

		case DEV_RAM:
			if(!ramdisk) stat|=STA_NODISK;
			return stat;

		case DEV_SD1:

			// TODO ###

			return stat;

        default: break;
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
			if(!ifs) {
				ifs=fopen("ifs.dat","a+b");
				if(ifs) {
					fseek(ifs,0,SEEK_END);
        			unsigned long l=ftell(ifs);
        			rewind(ifs);
					if(l<(IFS_SIZE*1024)) {	// create the IFS emulation file
						unsigned char *buff=NULL;
						xalloc((unsigned char **)&buff,1024);
						while(l<(IFS_SIZE*1024)) {
							fwrite(buff,1,1024,ifs);
							l+=1024;
						}
						xfree((unsigned char **)&buff);
					}
					fclose(ifs);
				}
				else stat|=STA_NODISK;
			}
			return stat;

		case DEV_RAM:
			if(!ramdisk) {
				xalloc((unsigned char **)&ramdisk,(1024*RAM_DISK_SIZE));    // allocate RAM disk memory
				if(!ramdisk) stat|=STA_NODISK;
			}
			return stat;

		case DEV_SD1:

			// TODO ###

			return stat;

        default: break;

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
			if(ifs) {
				if(sector<((IFS_SIZE*1024)/FatFs.ssize)) {
					ifs=fopen("ifs.dat","rb");
					if(ifs) {
						fseek(ifs,(sector*FatFs.ssize),SEEK_SET);
						fread(buff,FatFs.ssize,count,ifs);
						fclose(ifs);
					}
					else res=RES_NOTRDY;
				}
				else res=RES_PARERR;
			} else res=RES_NOTRDY;
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

			// TODO ###

			return res;

        default: break;

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
			if(ifs) {
				if(sector<((IFS_SIZE*1024)/FatFs.ssize)) {
					ifs=fopen("ifs.dat","r+b");
					if(ifs) {
						fseek(ifs,(sector*FatFs.ssize),SEEK_SET);
						fwrite(buff,FatFs.ssize,count,ifs);
						fclose(ifs);
					}
					else res=RES_NOTRDY;
				}
				else res=RES_PARERR;
			} else res=RES_NOTRDY;
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

			// TODO ###

			return res;

        default: break;

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
			switch(cmd) {
				case GET_SECTOR_COUNT:
					v=0;
					memcpy((unsigned char *)buff,(unsigned char *)&v,sizeof(DWORD));
					break;

				case GET_SECTOR_SIZE:
					h=FatFs.ssize;
					memcpy((unsigned char *)buff,(unsigned char *)&h,sizeof(WORD));
					break;

				case GET_BLOCK_SIZE:
					v=1;    // not flash memory
					memcpy((unsigned char *)buff,(unsigned char *)&v,sizeof(DWORD));
					break;

			}
			return res;

		case DEV_IFS:
			switch(cmd) {
				case GET_SECTOR_COUNT:
					v=(IFS_SIZE*1024)/FatFs.ssize;
					memcpy((unsigned char *)buff,(unsigned char *)&v,sizeof(DWORD));
					break;

				case GET_SECTOR_SIZE:
					h=FF_MIN_SS;
					memcpy((unsigned char *)buff,(unsigned char *)&h,sizeof(WORD));
					break;

				case GET_BLOCK_SIZE:
					v=1;    // not flash memory
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

			// TODO ###

			return res;

        default: break;
	}
	return RES_PARERR;
}
