//
// config.c
// Read the content of a ini file and set the configuration
//
// PiGFX is a bare metal kernel for the Raspberry Pi
// that implements a basic ANSI terminal emulator with
// the additional support of some primitive graphics functions.
// Copyright (C) 2020 Christian Lehner

#include "config.h"
#include "emmc.h"
#include "mbr.h"
#include "fat.h"
#include "ee_printf.h"
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
    else if (pigfx_strcmp(name, "disableGfxDMA") == 0)
    {
        tmpValue = atoi(value);
        if ((tmpValue == 0) || (tmpValue == 1)) PiGfxConfig.disableGfxDMA = tmpValue;
    }
    else if (pigfx_strcmp(name, "disableCollision") == 0)
    {
        tmpValue = atoi(value);
        if ((tmpValue == 0) || (tmpValue == 1)) PiGfxConfig.disableCollision = tmpValue;
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
    PiGfxConfig.disableGfxDMA = 1;
    PiGfxConfig.disableCollision = 0;
    pigfx_strcpy(PiGfxConfig.keyboardLayout, "us");
}

unsigned char lookForConfigFile(struct block_device *sd_dev)
{
    int retVal;

    struct fs * filesys = sd_dev->fs;
    if ((filesys == 0) || (sd_dev == 0))
    {
        ee_printf("Error reading file system\n");
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
    struct dirent* old_dirent = 0;
    while(1)
    {
        old_dirent = direntry;
        // look for configfile
        if (pigfx_strcmp(CONFIGFILENAME, direntry->name) == 0)
        {
            // File found
            configfileentry = direntry;
            if (direntry->next) direntry = direntry->next;
        }
        else if (direntry->next)
        {
            direntry = direntry->next;
            nmalloc_free(old_dirent);
        }
        else
        {
            nmalloc_free(old_dirent);
            break;
        }
    }
    if (configfileentry == 0)
    {
        ee_printf("Error locating config file\n");
        nmalloc_free(old_dirent);
        return errLOCFILE;
    }

    // read config file
    FILE *configfile = filesys->fopen(filesys, configfileentry, "r");
    if (configfile == 0)
    {
        ee_printf("Error opening config file\n");
        nmalloc_free(configfileentry);
        return errOPENFILE;
    }

    ee_printf("Found %s with length %d bytes\n", configfileentry->name, configfile->len);
    char* cfgfiledata = nmalloc_malloc(configfile->len+1);
    cfgfiledata[configfile->len] = 0;       // to be sure that this has a stringend somewhere
    if (filesys->fread(filesys, cfgfiledata, configfile->len, configfile) != (size_t)configfile->len)
    {
        ee_printf("Error reading config file\n");
        nmalloc_free(cfgfiledata);
        nmalloc_free(configfileentry);
        return errREADFILE;
    }

    // Interpret file content
    retVal = ini_parse_string(cfgfiledata, inihandler, 0);
    if (retVal < 0)
    {
        ee_printf("Syntax error %d interpreting config file\n", retVal);
        nmalloc_free(cfgfiledata);
        nmalloc_free(configfileentry);
        return errSYNTAX;
    }

    nmalloc_free(cfgfiledata);
    filesys->fclose(filesys, configfile);
    nmalloc_free(configfileentry);
    return errOK;
}
