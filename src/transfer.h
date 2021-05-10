//
// transfer.h
// File transfer to and from host
//
// PiGFX is a bare metal kernel for the Raspberry Pi
// that implements a basic ANSI terminal emulator with
// the additional support of some primitive graphics functions.
// Copyright (C) 2014-2021 Filippo Bergamasco, Christian Lehner

#ifndef _PIGFX_TRANSFER_H_
#define _PIGFX_TRANSFER_H_

#include "fat.h"
#include "nmalloc.h"
#include "c_utils.h"
#include "ee_printf.h"

#define TRANSFER_OK             0
#define TRANSFER_NUL_P          1
#define TRANSFER_READ_DIR_ERR   2
#define TRANSFER_NOT_FOUND      3
#define TRANSFER_ERR_OPEN       4
#define TRANSFER_ERR_READ       5

int fOpenTransferDir(struct block_device *sd_dev, struct dirent **direntry);
int fFreeDirMem(struct dirent* direntry);
int fLookForFile(struct dirent* direntry, char* filename, struct dirent **reqFile);
int fLoadFileToRam(struct block_device *sd_dev, struct dirent* direntry, char** ram);

#endif
