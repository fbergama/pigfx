#ifndef EMMC_H_
#define EMMC_H_

#include "block.h"

int sd_card_init(struct block_device **dev);

#endif
