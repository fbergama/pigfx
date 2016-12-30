#ifndef _LIBFS_H_
#define _LIBFS_H_


#define DIR unsigned int *
#define uint8_t unsigned char
#define uint32_t unsigned int
#define size_t uint32_t
#define FILE uint32_t *

struct dirent {
	struct dirent *next;
        char *name;
	uint32_t byte_size;
	uint8_t is_dir;
	void *opaque;
	struct fs *fs;
};

extern void libfs_init();
extern void *quick_memcpy(void *dest, void *src, unsigned int n);
extern DIR *opendir(const char *name);
extern struct dirent *readdir(DIR *dirp);
extern int closedir(DIR *dirp);
extern void vfs_list_devices();

extern FILE *fopen(const char *path, const char *mode);
extern int fclose(FILE *fp);
extern size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
extern size_t fwrite(void *ptr, size_t size, size_t nmemb, FILE *stream);

extern void memory_barrier();


extern int (*stdout_putc)(int);
extern int (*stderr_putc)(int);

#endif
