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
#include "fat.h"
#include "utils.h"
#include "c_utils.h"
#include "nmalloc.h"
#include "ee_printf.h"

#ifdef DEBUG2
#define FAT_DEBUG
#endif

struct fat_fs {
	struct fs b;
	int fat_type;
	uint32_t total_sectors;
	uint32_t sectors_per_cluster;
	uint32_t bytes_per_sector;
	char *vol_label;
	uint32_t first_fat_sector;
	uint32_t first_data_sector;
	uint32_t sectors_per_fat;
	uint32_t root_dir_entries;
	uint32_t root_dir_sectors;
	uint32_t first_non_root_sector;
	uint32_t root_dir_cluster;
};

// FAT32 extended fields
struct fat_extBS_32
{
	uint32_t		table_size_32;
	uint16_t		extended_flags;
	uint16_t		fat_version;
	uint32_t		root_cluster;
	uint16_t		fat_info;
	uint16_t		backup_BS_sector;
	uint8_t			reserved_0[12];
	uint8_t			drive_number;
	uint8_t			reserved_1;
	uint8_t			boot_signature;
	uint32_t		volume_id;
	char			volume_label[11];
	uint8_t			fat_type_label[8];
} __attribute__ ((packed));

// FAT 12/16 extended fields
struct fat_extBS_16
{
	uint8_t			bios_drive_num;
	uint8_t			reserved1;
	uint8_t			boot_signature;
	uint32_t		volume_id;
	char			volume_label[11];
	uint8_t			fat_type_label[8];
} __attribute__ ((packed));

// Generic FAT fields
struct fat_BS
{
	uint8_t			bootjmp[3];
	uint8_t			oem_name[8];
	uint16_t		bytes_per_sector;
	uint8_t			sectors_per_cluster;
	uint16_t		reserved_sector_count;
	uint8_t			table_count;
	uint16_t		root_entry_count;
	uint16_t		total_sectors_16;
	uint8_t			media_type;
	uint16_t		table_size_16;
	uint16_t		sectors_per_track;
	uint16_t		head_side_count;
	uint32_t		hidden_sector_count;
	uint32_t		total_sectors_32;

	union
	{
		struct fat_extBS_32	fat32;
		struct fat_extBS_16	fat16;
	} ext;
} __attribute__ ((packed));

#define FAT12		0
#define FAT16		1
#define FAT32		2
#define VFAT		3

static struct dirent *fat_read_dir(struct fat_fs *fs, struct dirent *d);
struct dirent *fat_read_directory(struct fs *fs, char **name);
static uint32_t fat_get_next_bdev_block_num(uint32_t f_block_idx, FILE *s, void *opaque, int add_blocks);

struct fat_file_block_offset
{
	uint32_t f_block;
	uint32_t cluster;
};

static const char *fat_names[] = { "FAT12", "FAT16", "FAT32", "VFAT" };

/* The fread/fwrite() functions in filesystems code shares a lot of common functionality
 * We provide that here
 * There are essentially two types of filesystem as regards to indexing blocks
 * within a file.
 * Assume a file contains n blocks and we want block i.
 * Filesystems like ext2, nofs can tell us the block number from i
 * Ones like FAT need to know the block number i - 1 and work it out from there
 *
 * Thus, if the block number can be calculated from i, can_index_blocks is set
 * to 1.
 *
 * fs_fread fills in as many of the parameters of get_next_block_num as it can
 */

size_t fs_fread(uint32_t (*get_next_bdev_block_num)(uint32_t f_block_idx, FILE *s, void *opaque, int add_blocks),
	struct fs *fs, void *ptr, size_t byte_size,
	FILE *stream, void *opaque)
{
	uint32_t fs_block_size = fs->block_size;

	// Determine first and last block indices within file
	uint32_t first_f_block_idx = stream->pos / fs_block_size;
	uint32_t first_f_block_offset = stream->pos % fs_block_size;
	uint32_t last_pos = stream->pos + byte_size;
	uint32_t last_f_block_idx = last_pos / fs_block_size;
	uint32_t last_f_block_offset = last_pos % fs_block_size;

	// Now iterate through the blocks
	uint32_t cur_block = first_f_block_idx;
	uint8_t *save_buf = (uint8_t *)ptr;
	int total_bytes_read = 0;
	while(cur_block <= last_f_block_idx)
	{
		uint32_t start_block_offset = 0;
		uint32_t last_block_offset = fs_block_size;

		// If we're the first block, adjust start_block_idx appropriately
		if(cur_block == first_f_block_idx)
			start_block_offset = first_f_block_offset;
		// If we're the last block, adjust last_block_idx appropriately
		if(cur_block == last_f_block_idx)
			last_block_offset = last_f_block_offset;

		uint32_t block_segment_length = last_block_offset - start_block_offset;

		// Get the filesystem block number
		uint32_t cur_bdev_block = get_next_bdev_block_num(cur_block, stream, opaque, 0);
		if(cur_bdev_block == 0xffffffff)
			return total_bytes_read;

		// If we can load an entire block, load it directly, else we have
		//  to load to a buffer somewhere and copy appropriately
		if((start_block_offset == 0) && (block_segment_length == fs_block_size))
		{
			int bytes_read = block_read(fs->parent, save_buf, fs_block_size, cur_bdev_block);
			total_bytes_read += bytes_read;
			stream->pos += bytes_read;
			save_buf += bytes_read;
			if((uint32_t)bytes_read != fs_block_size)
				return total_bytes_read;
		}
		else
		{
			// We have to load to a temporary buffer
			uint8_t *temp_buf = (uint8_t *)nmalloc_malloc(fs_block_size);
			int bytes_read = block_read(fs->parent, temp_buf, fs_block_size, cur_bdev_block);
			if(last_block_offset > (uint32_t)bytes_read)
				last_block_offset = bytes_read;
			if(last_block_offset < start_block_offset)
				block_segment_length = 0;
			else
				block_segment_length = last_block_offset - start_block_offset;

			// Copy from the temporary buffer to the save buffer
			qmemcpy(save_buf, &temp_buf[start_block_offset], block_segment_length);

			// Increment the pointers
			total_bytes_read += block_segment_length;
			stream->pos += block_segment_length;
			save_buf += block_segment_length;

			nmalloc_free(temp_buf);

			if((uint32_t)bytes_read != fs_block_size)
				return total_bytes_read;
		}

		cur_block++;
	}

	return total_bytes_read;
}

static FILE *fat_fopen(struct fs *fs, struct dirent *path, const char *mode)
{
	if(fs != path->fs)
	{
		return (FILE *)0;
	}

	if(pigfx_strcmp(mode, "r"))
	{
        // other than r
		return (FILE *)0;
	}

	struct vfs_file *ret = (struct vfs_file *)nmalloc_malloc(sizeof(struct vfs_file));
	pigfx_memset(ret, 0, sizeof(struct vfs_file));
	ret->fs = fs;
	ret->pos = 0;
	ret->opaque = path->opaque;
	ret->len = (long)path->byte_size;

	(void)mode;
	return ret;
}

static size_t fat_fread(struct fs *fs, void *ptr, size_t byte_size, FILE *stream)
{
	if(stream->fs != fs)
		return -1;
	if(stream->opaque == (void *)0)
		return -1;

	struct fat_file_block_offset opaque;
	opaque.cluster = (uintptr_t)stream->opaque;
	opaque.f_block = 0;
	return fs_fread(fat_get_next_bdev_block_num, fs, ptr, byte_size, stream, (void*)&opaque);
}

static int fat_fclose(struct fs *fs, FILE *fp)
{
	(void)fs;
	(void)fp;
	return 0;
}

int fat_init(struct block_device *parent, struct fs **fs)
{
	// Interpret a FAT file system
#ifdef FAT_DEBUG
	ee_printf("FAT: looking for a filesytem on %s\n", parent->device_name);
#endif

	// Read block 0
	uint8_t *block_0 = (uint8_t *)nmalloc_malloc(512);
	int r = block_read(parent, block_0, 512, 0);
	if(r < 0)
	{
		ee_printf("FAT: error %i reading block 0\n", r);
		return r;
	}
	if(r != 512)
	{
		ee_printf("FAT: error reading block 0 (only %i bytes read)\n", r);
		return -1;
	}

	// Dump the boot block
#ifdef FAT_DEBUG
	int j = 0;
	for(int i = 0; i < 90; i++)
	{
		ee_printf("%02x ", block_0[i]);
		j++;
		if(j == 8)
		{
			j = 0;
			ee_printf("\n");
		}
	}
	if(j != 0)
		ee_printf("\n");
#endif

	struct fat_BS *bs = (struct fat_BS *)block_0;
	if(bs->bootjmp[0] != 0xeb)
	{
		ee_printf("FAT: not a valid FAT filesystem on %s (%x)\n", parent->device_name,
				bs->bootjmp[0]);
		return -1;
	}
	uint32_t total_sectors = (uint32_t) read_halfword((unsigned char*)&bs->total_sectors_16, 0);
	if(total_sectors == 0)
		total_sectors = read_word((unsigned char*)&bs->total_sectors_32, 0);

	struct fat_fs *ret = (struct fat_fs *)nmalloc_malloc(sizeof(struct fat_fs));
	pigfx_memset(ret, 0, sizeof(struct fat_fs));
	ret->b.fopen = fat_fopen;
	ret->b.fread = fat_fread;
	ret->b.fclose = fat_fclose;
	ret->b.read_directory = fat_read_directory;
	ret->b.parent = parent;

	ret->total_sectors = total_sectors;

	ret->bytes_per_sector = (uint32_t)read_halfword((unsigned char*)&bs->bytes_per_sector,0);
	ret->root_dir_entries = read_halfword((unsigned char*)&bs->root_entry_count,0);
	ret->root_dir_sectors = (ret->root_dir_entries * 32 + ret->bytes_per_sector - 1) /
		ret->bytes_per_sector;	// The + bytes_per_sector - 1 rounds up the sector no

	uint32_t fat_size = read_halfword((unsigned char*)&bs->table_size_16, 0);
	if(fat_size == 0)
	    fat_size = read_word((unsigned char*)&bs->ext.fat32.table_size_32,0);

	uint32_t data_sec = total_sectors - (read_halfword((unsigned char*)&bs->reserved_sector_count,0) +
			bs->table_count * fat_size + ret->root_dir_sectors);

	uint32_t total_clusters = data_sec / bs->sectors_per_cluster;
	if(total_clusters < 4085)
		ret->fat_type = FAT12;
	else if(total_clusters < 65525)
		ret->fat_type = FAT16;
	else
		ret->fat_type = FAT32;
	ret->b.fs_name = fat_names[ret->fat_type];
	ret->sectors_per_cluster = (uint32_t)bs->sectors_per_cluster;

#ifdef FAT_DEBUG
	ee_printf("FAT: reading a %s filesystem: total_sectors %i, sectors_per_cluster %i, "
		       "bytes_per_sector %i\n",
	       ret->b.fs_name, ret->total_sectors, ret->sectors_per_cluster,
		ret->bytes_per_sector);
#endif

	// Interpret the extended bpb
	ret->vol_label = (char *)nmalloc_malloc(12);
	if(ret->fat_type == FAT32)
	{
		// FAT32
		pigfx_strncpy(ret->vol_label, bs->ext.fat32.volume_label, 11);
		ret->vol_label[11] = 0;
		ee_printf("FAT: volume label: %s\n", ret->vol_label);

		ret->first_data_sector = bs->reserved_sector_count + (bs->table_count *
			bs->ext.fat32.table_size_32);
		ret->first_fat_sector = bs->reserved_sector_count;
		ret->first_non_root_sector = ret->first_data_sector;
		ret->sectors_per_fat = bs->ext.fat32.table_size_32;

#ifdef FAT_DEBUG
		ee_printf("FAT: first_data_sector: %i, first_fat_sector: %i\n",
				ret->first_data_sector,
				ret->first_fat_sector);
#endif

		ret->root_dir_cluster = bs->ext.fat32.root_cluster;
	}
	else
	{
		// FAT12/16

		pigfx_strncpy(ret->vol_label, bs->ext.fat16.volume_label, 11);
		ret->vol_label[11] = 0;
#ifdef FAT_DEBUG
		ee_printf("FAT: volume label: %s\n", ret->vol_label);
#endif

		ret->first_data_sector = bs->reserved_sector_count + (bs->table_count *
				bs->table_size_16);
		ret->first_fat_sector = bs->reserved_sector_count;
		ret->sectors_per_fat = bs->table_size_16;

#ifdef FAT_DEBUG
		ee_printf("FAT: first_data_sector: %i, first_fat_sector: %i\n",
				ret->first_data_sector,
				ret->first_fat_sector);

		ee_printf("FAT: root_dir_entries: %i, root_dir_sectors: %i\n",
				ret->root_dir_entries,
				ret->root_dir_sectors);
#endif

		ret->first_non_root_sector = ret->first_data_sector + ret->root_dir_sectors;
		ret->root_dir_cluster = 2;
	}

	ret->b.block_size = ret->bytes_per_sector * ret->sectors_per_cluster;
	*fs = (struct fs *)ret;
	nmalloc_free(block_0);

	ee_printf("FAT: found a %s filesystem on %s\n", ret->b.fs_name, ret->b.parent->device_name);

	return 0;
}

uint32_t get_sector(struct fat_fs *fs, uint32_t rel_cluster)
{
#ifdef FAT_DEBUG
	ee_printf("FAT: get_sector rel_cluster %i, sector %i\n",
			rel_cluster,
			fs->first_non_root_sector + (rel_cluster - 2) * fs->sectors_per_cluster);
#endif
	rel_cluster -= 2;
	return fs->first_non_root_sector + rel_cluster * fs->sectors_per_cluster;
}

static uint32_t get_next_fat_entry(struct fat_fs *fs, uint32_t current_cluster)
{
	switch(fs->fat_type)
	{
		case FAT16:
			{
				uint32_t fat_offset = current_cluster << 1; // *2
				uint32_t fat_sector = fs->first_fat_sector +
					(fat_offset / fs->bytes_per_sector);
				uint8_t *buf = (uint8_t *)nmalloc_malloc(512);
				int br_ret = block_read(fs->b.parent, buf, 512, fat_sector);
				if(br_ret < 0)
				{
					ee_printf("FAT: block_read returned %i\n");
					return 0x0ffffff7;
				}
				uint32_t fat_index = fat_offset % fs->bytes_per_sector;
				uint32_t next_cluster = (uint32_t)*(uint16_t *)&buf[fat_index];
				nmalloc_free(buf);
				if(next_cluster >= 0xfff7)
					next_cluster |= 0x0fff0000;
				return next_cluster;
			}

		case FAT32:
			{
				uint32_t fat_offset = current_cluster << 2; // *4
				uint32_t fat_sector = fs->first_fat_sector +
					(fat_offset / fs->bytes_per_sector);
				uint8_t *buf = (uint8_t *)nmalloc_malloc(512);
				int br_ret = block_read(fs->b.parent, buf, 512, fat_sector);
				if(br_ret < 0)
				{
					ee_printf("FAT: block_read returned %i\n");
					return 0x0ffffff7;
				}
				uint32_t fat_index = fat_offset % fs->bytes_per_sector;
				uint32_t next_cluster = *(uint32_t *)&buf[fat_index];
				nmalloc_free(buf);
				return next_cluster & 0x0fffffff; // FAT32 is actually FAT28
			}
		default:
			ee_printf("FAT: fat type %s not supported\n", fs->b.fs_name);
			return 0;
	}
}

struct dirent *fat_read_directory(struct fs *fs, char **name)
{
	struct dirent *cur_dir = fat_read_dir((struct fat_fs *)fs, (void*)0);
	while(*name)
	{
		// Search the directory entries for one of the requested name
		int found = 0;
		while(cur_dir)
		{
            ee_printf("name1: %s, name2: %s\n", *name, cur_dir->name);
			if(!pigfx_strcmp(*name, cur_dir->name))
			{
				if(!cur_dir->is_dir)
				{
					return (void*)0;
				}
				found = 1;
				cur_dir = fat_read_dir((struct fat_fs *)fs, cur_dir);
				name++;
				break;
			}
			cur_dir = cur_dir->next;
		}
		if(!found)
		{
#ifdef FAT_DEBUG
			ee_printf("FAT: path part %s not found\n", *name);
#endif
			return (void*)0;
		}
	}
	return cur_dir;
}

static uint32_t fat_get_next_bdev_block_num(uint32_t f_block_idx, FILE *s, void *opaque, int add_blocks)
{
	struct fat_file_block_offset *ffbo = (struct fat_file_block_offset *)opaque;

	// Iterate through the cluster chain until we reach the appropriate one
	while((ffbo->f_block != f_block_idx) && (ffbo->cluster < 0x0ffffff8))
	{
		ffbo->cluster = get_next_fat_entry((struct fat_fs *)s->fs, ffbo->cluster);
		ffbo->f_block++;
	}

	if(ffbo->cluster < 0x0ffffff8)
		return get_sector((struct fat_fs *)s->fs, ffbo->cluster);
	else
	{
		if(add_blocks)
		{
			ee_printf("FAT: request to extend cluster chain not currently supported\n");
		}
		s->flags |= 1;        // EOF Flag
		return 0xffffffff;
	}
}

struct dirent *fat_read_dir(struct fat_fs *fs, struct dirent *d)
{
	int is_root = 0;
	struct fat_fs *fat = (struct fat_fs *)fs;

	if(d == (void*)0)
		is_root = 1;

	uint32_t cur_cluster;
	uint32_t cur_root_cluster_offset = 0;
	if(is_root)
		cur_cluster = fat->root_dir_cluster;
	else
		cur_cluster = (uintptr_t)d->opaque;

	struct dirent *ret = (void *)0;
	struct dirent *prev = (void *)0;

#ifdef FAT_DEBUG
	ee_printf("FAT: read_dir: starting directory read from cluster %i\n", cur_cluster);
#endif

	do
	{
		/* Read this cluster */
		uint32_t cluster_size = fat->bytes_per_sector * fat->sectors_per_cluster;
		uint8_t *buf = (uint8_t *)nmalloc_malloc(cluster_size);

		/* Interpret the cluster number to an absolute address */
		uint32_t absolute_cluster = cur_cluster - 2;
		uint32_t first_data_sector = fat->first_data_sector;
		if(!is_root)
			first_data_sector = fat->first_non_root_sector;

#ifdef FAT_DEBUG
		ee_printf("FAT: reading cluster %i (sector %i)\n", cur_cluster,
				absolute_cluster * fat->sectors_per_cluster + first_data_sector);
#endif
		int br_ret = block_read(fat->b.parent, buf, cluster_size,
				absolute_cluster * fat->sectors_per_cluster + first_data_sector);

		if(br_ret < 0)
		{
			ee_printf("FAT: block_read returned %i\n", br_ret);
			return (void*)0;
		}

		for(uint32_t ptr = 0; ptr < cluster_size; ptr += 32)
		{
			// Does the entry exist (if the first byte is zero or 0xe5 it doesn't)
			if((buf[ptr] == 0) || (buf[ptr] == 0xe5))
				continue;

			// Is it the directories '.' or '..'?
			if(buf[ptr] == '.' && buf[ptr + 1] == ' ')
				continue;
			if(buf[ptr] == '.' && buf[ptr + 1] == '.' && buf[ptr + 2] == ' ')
				continue;

			// Is it the volume label or a long filename entry (if so ignore)
			if(buf[ptr + 11] & 0x08)
				continue;

			// Else read it
			struct dirent *de = (struct dirent *)nmalloc_malloc(sizeof(struct dirent));
			pigfx_memset(de, 0, sizeof(struct dirent));
			if(ret == (void *)0)
				ret = de;
			if(prev != (void *)0)
				prev->next = de;
			prev = de;

			de->name = (char *)nmalloc_malloc(13);
			de->fs = &fs->b;
			// Convert to lowercase on load
			int d_idx = 0;
			int in_ext = 0;
			int has_ext = 0;
			for(int i = 0; i < 11; i++)
			{
				char cur_v = (char)buf[ptr + i];
				if(i == 8)
				{
					in_ext = 1;
					de->name[d_idx++] = '.';
				}
				if(cur_v == ' ')
					continue;
				if(in_ext)
					has_ext = 1;
				if((cur_v >= 'A') && (cur_v <= 'Z'))
					cur_v = 'a' + cur_v - 'A';
				de->name[d_idx++] = cur_v;
			}
			if(!has_ext)
				de->name[d_idx - 1] = 0;
			else
				de->name[d_idx] = 0;

			if(buf[ptr + 11] & 0x10)
				de->is_dir = 1;
			de->next = (void *)0;
			de->byte_size = read_word(buf, ptr + 28);
			uintptr_t opaque = read_halfword(buf, ptr + 26) |
				((uint32_t)read_halfword(buf, ptr + 20) << 16);

			de->opaque = (void*)opaque;

#ifdef FAT_DEBUG
			ee_printf("FAT: read dir entry: %s, size %i, cluster %i, ptr %i\n",
					de->name, de->byte_size, opaque, ptr);
#endif
		}
		nmalloc_free(buf);

		// Get the next cluster
		if(is_root && (fs->fat_type != FAT32))
		{
			cur_root_cluster_offset++;
			if(cur_root_cluster_offset < (fat->root_dir_sectors /
						fat->sectors_per_cluster))
				cur_cluster++;
			else
				cur_cluster = 0x0ffffff8;
		}
		else
			cur_cluster = get_next_fat_entry(fat, cur_cluster);

#ifdef FAT_DEBUG
		ee_printf("FAT: read dir: next cluster %x\n", cur_cluster);
#endif
	} while(cur_cluster < 0x0ffffff7);

	return ret;
}

