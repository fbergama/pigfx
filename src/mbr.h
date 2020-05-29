#ifndef MBR_H_
#define MBR_H_

#include "block.h"

int read_mbr(struct block_device *parent, struct block_device ***partitions, int *part_count);

#endif
 
