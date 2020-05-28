#ifndef FAT_H__
#define FAT_H__ 

#include "block.h"

#ifdef FILE
#undef FILE
#endif
#define FILE struct vfs_file

struct dirent {
	struct dirent *next;
        char *name;
	uint32_t byte_size;
	uint8_t is_dir;
	void *opaque;
	struct fs *fs;
};

struct vfs_file
{
    struct fs *fs;
    long pos;
	int mode;
    void *opaque;
    long len;
	int flags;
	int (*fflush_cb)(FILE *f);
};

struct fs {
	struct block_device *parent;
	const char *fs_name;
	uint32_t flags;
	size_t block_size;

	FILE *(*fopen)(struct fs *, struct dirent *, const char *mode);
	size_t (*fread)(struct fs *, void *ptr, size_t byte_size, FILE *stream);
	size_t (*fwrite)(struct fs *, void *ptr, size_t byte_size, FILE *stream);
	int (*fclose)(struct fs *, FILE *fp);
	long (*fsize)(FILE *fp);
    int (*fseek)(FILE *stream, long offset, int whence);
	long (*ftell)(FILE *fp);
	int (*fflush)(FILE *fp);

	struct dirent *(*read_directory)(struct fs *, char **name);
};

int register_fs(struct block_device *dev, int part_id);
int fs_interpret_mode(const char *mode);
size_t fs_fread(uint32_t (*get_next_bdev_block_num)(uint32_t f_block_idx, FILE *s, void *opaque, int add_blocks),
	struct fs *fs, void *ptr, size_t byte_size,
	FILE *stream, void *opaque);
size_t fs_fwrite(uint32_t (*get_next_bdev_block_num)(uint32_t f_block_idx, FILE *s, void *opaque, int add_blocks),
	struct fs *fs, void *ptr, size_t byte_size,
	FILE *stream, void *opaque);

int fat_init(struct block_device *parent, struct fs **fs);

#endif 
