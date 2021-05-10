//
// transfer.c
// File transfer to and from host
//
// PiGFX is a bare metal kernel for the Raspberry Pi
// that implements a basic ANSI terminal emulator with
// the additional support of some primitive graphics functions.
// Copyright (C) 2014-2021 Filippo Bergamasco, Christian Lehner
#include "transfer.h"

int fOpenTransferDir(struct block_device *sd_dev, struct dirent **direntry)
{
    if (sd_dev == 0) return TRANSFER_NUL_P;  // sd_dev invalid

    struct fs * filesys = sd_dev->fs;
    if (filesys == 0) return TRANSFER_NUL_P;    // no file device

    char* mydirname[2];
    mydirname[0] = "transfer";
    mydirname[1] = 0;
    *direntry = filesys->read_directory(filesys, mydirname);
    if (*direntry) return TRANSFER_OK;

    return TRANSFER_READ_DIR_ERR;
}

int fFreeDirMem(struct dirent* direntry)
{
    if (direntry == 0) return TRANSFER_NUL_P;
    struct dirent* actDir;
    while (1)
    {
        actDir = direntry;
        if (actDir)
        {
            direntry = direntry->next;
            nmalloc_free(actDir);
        }
        else return TRANSFER_OK;
    }
}

int fLookForFile(struct dirent* direntry, char* filename, struct dirent **reqFile)
{
    if (direntry == 0) return TRANSFER_NUL_P;

    while (1)
    {
        if (pigfx_strcmp(filename, direntry->name) == 0)
        {
            *reqFile = direntry;
            return TRANSFER_OK;
        }
        if (direntry->next) direntry = direntry->next;
        else break;
    }
    return TRANSFER_NOT_FOUND;
}

int fLoadFileToRam(struct block_device *sd_dev, struct dirent* direntry, char** ram)
{
    if (sd_dev == 0) return TRANSFER_NUL_P;  // sd_dev invalid
    if (direntry == 0) return TRANSFER_NUL_P;

    struct fs * filesys = sd_dev->fs;
    if (filesys == 0) return TRANSFER_NUL_P;    // no file device

    FILE *myFile = filesys->fopen(filesys, direntry, "r");
    if (myFile == 0) return TRANSFER_ERR_OPEN;

    *ram = nmalloc_malloc(myFile->len+1);
    *ram[myFile->len] = 0;  // stringterm
    if (filesys->fread(filesys, *ram, myFile->len, myFile) != (size_t)myFile->len)
    {
        nmalloc_free(myFile);
        nmalloc_free(*ram);
        return TRANSFER_ERR_READ;
    }
    return TRANSFER_OK;
}
