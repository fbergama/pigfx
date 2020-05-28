/* Copyright Christian Lehner 2020
 * PiGFX Project
 *
 */

#include "config.h"
#include "emmc.h"
#include "mbr.h"
#include "fat.h"
#include "ee_printf.h"
#include "block.h"
#include "nmalloc.h"
#include "c_utils.h"

void setDefaultConfig()
{
    pigfx_memset(&PiGfxConfig, 0, sizeof(PiGfxConfig));
    
    PiGfxConfig.uartBaudrate = defUARTBAUDRATE;
    PiGfxConfig.useUsbKeyboard = defUSEUSBKEYBOARD;
}

unsigned char lookForConfigFile()
{
    struct block_device *sd_dev = 0;
    
    if(sd_card_init(&sd_dev) != 0)
    {
        ee_printf("Error initializing SD card\n");
        return errSDCARDINIT;
    }
    
    if ((read_mbr(sd_dev, (void*)0, (void*)0)) != 0)
    {
        ee_printf("Error reading MasterBootRecord\n");
        return errMBR;
    }
    
    struct fs * filesys = sd_dev->fs;
    if (filesys == 0)
    {
        ee_printf("Error reading filesystem\n");
        return errFS;
    }
    
    // loading root dir
    char* myfilename = 0;
    struct dirent *direntry = filesys->read_directory(filesys, &myfilename);
    if (direntry == 0)
    {
        ee_printf("Error reading root directory\n");
        return errREADROOT;
    }

    struct dirent * configfileentry = 0;
    while(1)
    {
        // look for configfile
        if (pigfx_strcmp(CONFIGFILENAME, direntry->name) == 0)
        {
            // File found
            configfileentry = direntry;
            break;
        }
        if (direntry->next) direntry = direntry->next;
        else break;
    }
    if (configfileentry == 0)
    {
        ee_printf("Error locating config file\n");
        return errLOCFILE;
    }

    // read config file
    FILE *configfile = filesys->fopen(filesys, configfileentry, "r");
    if (configfile == 0)
    {
        ee_printf("Error opening config file\n");
        return errOPENFILE;
    }

    ee_printf("Found %s with length %d bytes\n", configfileentry->name, configfile->len);
    char* cfgfiledata = nmalloc_malloc(configfile->len+1);
    cfgfiledata[configfile->len] = 0;       // to be sure that this has a stringend somewhere
    if (filesys->fread(filesys, cfgfiledata, configfile->len, configfile))
    {
        /*ee_printf(cfgfiledata);
        ee_printf("\n");*/
    }

    return errOK;
}
