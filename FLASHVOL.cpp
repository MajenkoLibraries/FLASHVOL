#include <FLASHVOL.h>

/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS FLASHVOL::disk_status(void)
{
    return(_status);
}

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS FLASHVOL::disk_initialize(void)
{
    _status &= ~STA_NOINIT;
    return(_status);
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT FLASHVOL::disk_read(
	uint8_t *buff,		/* Pointer to the data buffer to store read data */
	uint32_t sector,	/* Start sector number (LBA) */
	uint32_t count		/* Sector count (1..128) */
)
{
    if(_status &= STA_NOINIT)
    {
        return(RES_NOTRDY);
    }

    if(sector >= _sectors)
    {
        return(RES_PARERR);  
    }

    if((sector + count) > _sectors)
    {
        return(RES_PARERR);  
    }

    uint32_t offset = sector * CBSECTOR;
    uint32_t toRead  = count * CBSECTOR;
    uint8_t *vol8 = (uint8_t *)_vol;

    uint32_t *buff32 = (uint32_t *)buff;

    for (int i = 0; i < toRead; i++) {
        buff[i] = vol8[offset + i];
    }

    return(RES_OK); 
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT FLASHVOL::disk_write (
	const uint8_t *buff,		/* Pointer to the data to be written */
	uint32_t sector,			/* Start sector number (LBA) */
	uint32_t count				/* Sector count (1..128) */
)
{
    if(_status &= STA_NOINIT)
    {
        return(RES_NOTRDY);
    }

    if((sector + count) > _sectors)
    {
        return(RES_PARERR);  
    }

    uint32_t offset = sector * CBSECTOR / 4;
    uint32_t toWrite  = count * CBSECTOR / 4;
    uint32_t *buff32 = (uint32_t *)buff;

    const uint32_t *pageStart = &_vol[offset & ~(_EEPROM_PAGE_SIZE-1)];
    Flash.loadPage((void *)pageStart);

    for (int i = 0; i < toWrite; i++) {
        const uint32_t *ps = &_vol[(offset + i) & ~(_EEPROM_PAGE_SIZE-1)];
        if (ps != pageStart) {
            Flash.savePage();
            Flash.loadPage((void *)ps);
            pageStart = ps;
        }
        Flash.writePageWord((void *)&_vol[offset + i], buff32[i]);
    }

    Flash.savePage();

    return(RES_OK); 
}
#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT FLASHVOL::disk_ioctl (
	uint8_t cmd,		/* Control code */
	void *buff		/* Buffer to send/receive data block */
)
{
    switch(cmd)
    {
        /* Flush write-back cache, Wait for end of internal process */
        /* Complete pending write process (needed at _FS_READONLY == 0) */
        case CTRL_SYNC:
            return(RES_OK);
            break;

        /* Get media size (needed at _USE_MKFS == 1) */
        /* Get number of sectors on the disk (WORD) */
        case GET_SECTOR_COUNT:
            *(uint32_t*) buff = _sectors;
            return(RES_OK);
            break;

        /* Get sector size (needed at _MAX_SS != _MIN_SS) */
        case GET_SECTOR_SIZE:
            *(uint32_t*) buff = CBSECTOR;
            return(RES_OK);
            break;

        /* Get erase block size in unit of sectors (DWORD) */
        /* Get erase block size (needed at _USE_MKFS == 1) */
        case GET_BLOCK_SIZE:	
            *(uint32_t*) buff = (_EEPROM_PAGE_SIZE * 4) / CBSECTOR;
            return(RES_OK);
           break;

        /* Inform device that the data on the block of sectors is no longer used (needed at _USE_TRIM == 1) */
        case CTRL_TRIM:	
        default:
            return(RES_PARERR);
        break;
    }

    return(RES_PARERR);
}
#endif
