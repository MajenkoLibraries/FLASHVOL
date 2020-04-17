#ifndef _RAMVOL_H
#define _RAMVOL_H

#include <Arduino.h>
#include <DFATFS.h>
#include <Flash.h>

class FLASHVOL : public DFSVOL
{
    private:

        const uint32_t *_vol;
        uint32_t _sectors;
        DSTATUS  _status;
 
        // make default constructor illegal to use
        FLASHVOL();

    public:

        static const uint32_t CBSECTOR = _MIN_SS;
        static const uint32_t CMINSECTORS = 128;

        FLASHVOL(const uint32_t *vol, uint32_t sectors) : DFSVOL(1,1), _vol(vol), _sectors(sectors)
        {
            _status = STA_NOINIT; 
        }

        DSTATUS disk_initialize (void);
        DSTATUS disk_status (void);
        DRESULT disk_read (uint8_t* buff, uint32_t sector, uint32_t count);
        DRESULT disk_write (const uint8_t* buff, uint32_t sector, uint32_t count);
        DRESULT disk_ioctl (uint8_t cmd, void* buff);
};

#endif

