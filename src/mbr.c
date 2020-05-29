/* Copyright (C) 2013 by John Cronin <jncronin@tysos.org>
 * Modified to fit PiGFX by Chrisian Lehner 2020
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdint.h>
#include "nmalloc.h"
#include "ee_printf.h"
#include "mbr.h"
#include "utils.h"
#include "c_utils.h"
#include "fat.h"

#ifdef DEBUG2
#define MBR_DEBUG
#endif

// Code for interpreting an mbr

struct mbr_block_dev {
	struct block_device bd;
	struct block_device *parent;
	int part_no;
	uint32_t start_block;
	uint32_t blocks;
	uint8_t part_id;
};

static char driver_name[] = "mbr";

static int mbr_read(struct block_device *, uint8_t *buf, size_t buf_size, uint32_t starting_block);
static int mbr_write(struct block_device *, uint8_t *buf, size_t buf_size, uint32_t starting_block);

int register_fs(struct block_device *dev, int part_id)
{
	switch(part_id)
	{
		case 0:
			// Try reading it as an mbr, then try all known filesystems
			if(read_mbr(dev, 0, 0) == 0)
				break;
			if(fat_init(dev, &dev->fs) == 0)
				break;
			break;

		case 1:
		case 4:
		case 6:
		case 0xb:
		case 0xc:
		case 0xe:
		case 0x11:
		case 0x14:
		case 0x1b:
		case 0x1c:
		case 0x1e:
			fat_init(dev, &dev->fs);
			break;

		case 0x83:
            // ext2 not supported
			break;

		case 0xda:
            // nofs not supported
			break;
	}
	return 0;
}

int read_mbr(struct block_device *parent, struct block_device ***partitions, int *part_count)
{
	(void)partitions;
	(void)part_count;
	(void)driver_name;
	/* Check the validity of the parent device */
	if(parent == (void*)0)
	{
		ee_printf("MBR: invalid parent device\n");
		return -1;
	}

	/* Read the first 512 bytes */
	uint8_t *block_0 = (uint8_t *)nmalloc_malloc(512);
	if(block_0 == 0)
		return -1;

#ifdef MBR_DEBUG
	ee_printf("MBR: reading block 0 from device %s\n", parent->device_name);
#endif

	int ret = block_read(parent, block_0, 512, 0);
	if(ret < 0)
	{
		ee_printf("MBR: block_read failed (%i)\n", ret);
		nmalloc_free((void**)&block_0);
		return ret;
	}
	if(ret != 512)
	{
		ee_printf("MBR: unable to read first 512 bytes of device %s, only %d bytes read\n",
				parent->device_name, ret);
		nmalloc_free((void**)&block_0);
		return -1;
	}

	/* Check the MBR signature */
	if((block_0[0x1fe] != 0x55) || (block_0[0x1ff] != 0xaa))
	{
		ee_printf("MBR: no valid mbr signature on device %s (bytes are %x %x)\n",
				parent->device_name, block_0[0x1fe], block_0[0x1ff]);
		nmalloc_free((void**)&block_0);
		return -1;
	}
	ee_printf("MBR: found valid MBR on device %s\n", parent->device_name);

#ifdef MBR_DEBUG
	/* Dump the first sector */
	ee_printf("MBR: first sector:");
	for(int dump_idx = 0; dump_idx < 512; dump_idx++)
	{
		if((dump_idx & 0xf) == 0)
			ee_printf("\n%03x: ", dump_idx);
		ee_printf("%02x ", block_0[dump_idx]);
	}
	ee_printf("\n");
#endif

    // If parent block size is not 512, we have to coerce start_block
    //  and blocks to fit
    if(parent->block_size < 512)
    {
        // We do not support parent device block sizes < 512
        ee_printf("MBR: parent block device is too small (%i)\n", parent->block_size);
		nmalloc_free((void**)&block_0);
        return -1;
    }

    uint32_t block_size_adjust = parent->block_size / 512;
    if(parent->block_size % 512)
    {
        // We do not support parent device block sizes that are not a
        //  multiple of 512
        ee_printf("MBR: parent block size is not a multiple of 512 (%i)\n",
               parent->block_size);
		nmalloc_free((void**)&block_0);
        return -1;
    }

#ifdef MBR_DEBUG
    if(block_size_adjust > 1)
    {
        ee_printf("MBR: block_size_adjust: %i\n", block_size_adjust);
    }
#endif

	/* Load the partitions */
	struct block_device **parts =
		(struct block_device **)nmalloc_malloc(4 * sizeof(struct block_device *));
	int cur_p = 0;
	for(int i = 0; i < 4; i++)
	{
		int p_offset = 0x1be + (i * 0x10);
		if(block_0[p_offset + 4] != 0x00)
		{
			// Valid partition
			struct mbr_block_dev *d =
				(struct mbr_block_dev *)nmalloc_malloc(sizeof(struct mbr_block_dev));
			pigfx_memset(d, 0, sizeof(struct mbr_block_dev));
			d->bd.driver_name = driver_name;
			char *dev_name = (char *)nmalloc_malloc(pigfx_strlen(parent->device_name) + 3);
			pigfx_strcpy(dev_name, parent->device_name);
			dev_name[pigfx_strlen(parent->device_name)] = '_';
			dev_name[pigfx_strlen(parent->device_name) + 1] = '0' + i;
			dev_name[pigfx_strlen(parent->device_name) + 2] = 0;
			d->bd.device_name = dev_name;
			d->bd.device_id = (uint8_t *)nmalloc_malloc(1);
			d->bd.device_id[0] = i;
			d->bd.dev_id_len = 1;
			d->bd.read = mbr_read;
			if(parent->write)
                d->bd.write = mbr_write;
			d->bd.block_size = parent->block_size;
			d->bd.supports_multiple_block_read = parent->supports_multiple_block_read;
			d->bd.supports_multiple_block_write = parent->supports_multiple_block_write;
			d->part_no = i;
			d->part_id = block_0[p_offset + 4];
			d->start_block = read_word(block_0, p_offset + 8);
			d->blocks = read_word(block_0, p_offset + 12);
			d->parent = parent;

            // Adjust start_block and blocks to the parent block size
			if(d->start_block % block_size_adjust)
			{
			    ee_printf("MBR: partition number %i does not start on a block "
                    "boundary (%i).\n", d->part_no, d->start_block);
                return -1;
			}
			d->start_block /= block_size_adjust;

			if(d->blocks % block_size_adjust)
			{
			    ee_printf("MBR: partition number %i does not have a length "
                    "that is an exact multiple of the block length (%i).\n",
                    d->part_no, d->start_block);
                return -1;
			}
			d->blocks /= block_size_adjust;
			d->bd.num_blocks = d->blocks;

			parts[cur_p++] = (struct block_device *)d;
#ifdef MBR_DEBUG
			ee_printf("MBR: partition number %i (%s) of type %x, start sector %u, "
					"sector count %u, p_offset %03x\n",
					d->part_no, d->bd.device_name, d->part_id,
					d->start_block, d->blocks, p_offset);
#endif
			// Register the fs
			register_fs((struct block_device *)d, d->part_id);
            if ((i == 0) && (d->bd.fs))
            {
                // use first partition as active filesystem
                parent->fs = d->bd.fs;
            }
		}
	}

	if (0 != partitions)
		*partitions = parts;
	else
		nmalloc_free((void**)&parts);
	if (0 != part_count)
		*part_count = cur_p;
	ee_printf("MBR: found total of %i partition(s)\n", cur_p);

	nmalloc_free((void**)&block_0);

	return 0;
}

int mbr_read(struct block_device *dev, uint8_t *buf, size_t buf_size, uint32_t starting_block)
{
	struct block_device *parent = ((struct mbr_block_dev *)dev)->parent;

	return parent->read(parent, buf, buf_size,
			starting_block + ((struct mbr_block_dev *)dev)->start_block);
}

int mbr_write(struct block_device *dev, uint8_t *buf, size_t buf_size, uint32_t starting_block)
{
    struct block_device *parent = ((struct mbr_block_dev *)dev)->parent;

    return parent->write(parent, buf, buf_size,
                         starting_block + ((struct mbr_block_dev *)dev)->start_block);
}
