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
#include "ini.h"

int inihandler(void* user, const char* section, const char* name, const char* value)
{
    int tmpValue;
    
    (void)user;
    (void)section;      // we don't care about the section
    
    if (pigfx_strcmp(name, "baudrate") == 0)
    {
        tmpValue = atoi(value);
        if (tmpValue > 0) PiGfxConfig.uartBaudrate = tmpValue;
    }
    else if (pigfx_strcmp(name, "useUsbKeyboard") == 0)
    {
        tmpValue = atoi(value);
        if ((tmpValue == 0) || (tmpValue == 1)) PiGfxConfig.useUsbKeyboard = tmpValue;
    }
    else if (pigfx_strcmp(name, "sendCRLF") == 0)
    {
        tmpValue = atoi(value);
        if ((tmpValue == 0) || (tmpValue == 1)) PiGfxConfig.sendCRLF = tmpValue;
    }
    else if (pigfx_strcmp(name, "replaceLFwithCR") == 0)
    {
        tmpValue = atoi(value);
        if ((tmpValue == 0) || (tmpValue == 1)) PiGfxConfig.replaceLFwithCR = tmpValue;
    }
    else if (pigfx_strcmp(name, "backspaceEcho") == 0)
    {
        tmpValue = atoi(value);
        if ((tmpValue == 0) || (tmpValue == 1)) PiGfxConfig.backspaceEcho = tmpValue;
    }
    else if (pigfx_strcmp(name, "skipBackspaceEcho") == 0)
    {
        tmpValue = atoi(value);
        if ((tmpValue == 0) || (tmpValue == 1)) PiGfxConfig.skipBackspaceEcho = tmpValue;
    }
    else if (pigfx_strcmp(name, "swapDelWithBackspace") == 0)
    {
        tmpValue = atoi(value);
        if ((tmpValue == 0) || (tmpValue == 1)) PiGfxConfig.swapDelWithBackspace = tmpValue;
    }
    else if (pigfx_strcmp(name, "showRC2014Logo") == 0)
    {
        tmpValue = atoi(value);
        if ((tmpValue == 0) || (tmpValue == 1)) PiGfxConfig.showRC2014Logo = tmpValue;
    }
    else if (pigfx_strcmp(name, "keyboardLayout") == 0)
    {
        pigfx_strncpy(PiGfxConfig.keyboardLayout, value, sizeof(PiGfxConfig.keyboardLayout));
    }
    return 0;
}

void setDefaultConfig()
{
    pigfx_memset(&PiGfxConfig, 0, sizeof(PiGfxConfig));
    
    PiGfxConfig.uartBaudrate = 115200;
    PiGfxConfig.useUsbKeyboard = 1;
    PiGfxConfig.sendCRLF = 0;
    PiGfxConfig.replaceLFwithCR = 0;
    PiGfxConfig.backspaceEcho = 0;
    PiGfxConfig.skipBackspaceEcho = 0;
    PiGfxConfig.swapDelWithBackspace = 1;
    PiGfxConfig.showRC2014Logo = 0;
    pigfx_strcpy(PiGfxConfig.keyboardLayout, "us");
}

unsigned char lookForConfigFile()
{
    int retVal;
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
    if (filesys->fread(filesys, cfgfiledata, configfile->len, configfile) != (size_t)configfile->len)
    {
        ee_printf("Error reading config file\n");
        nmalloc_free((void**)&cfgfiledata);
        return errREADFILE;
    }
    
    // Interpret file content
    retVal = ini_parse_string(cfgfiledata, inihandler, 0);
    if (retVal < 0)
    {
        ee_printf("Syntax error %d interpreting config file\n", retVal);
        nmalloc_free((void**)&cfgfiledata);
        return errSYNTAX;
    }
    
    nmalloc_free((void**)&cfgfiledata);
    return errOK;
}
