#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}
extern size_t serial_write(const void *buf, size_t offset, size_t len);
extern size_t events_read(void *buf, size_t offset, size_t len);
extern size_t dispinfo_read(void *buf, size_t offset, size_t len);
extern size_t fb_write(const void *buf, size_t offset, size_t len);
size_t am_ioe_read(void *buf, size_t offset, size_t len);
size_t am_ioe_write(const void *buf, size_t offset, size_t len);
/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
  [FD_FB] = {"/dev/fb", 0, 0, invalid_read, fb_write},
  {"/dev/events", 0, 0, events_read, invalid_write},
  {"/proc/dispinfo", 0, 0, dispinfo_read, invalid_write},
  {"/dev/am_ioe", 128, 0, am_ioe_read, am_ioe_write}, //size = ?
#include "files.h"
};

void init_fs() {
  // TODO: initialize the size of /dev/fb
  file_table[FD_FB].size = io_read(AM_GPU_CONFIG).vmemsz;
  // printf("fb size: %d\n", file_table[FD_FB].size);
}

extern size_t ramdisk_read(void *buf, size_t offset, size_t len);
extern size_t ramdisk_write(const void *buf, size_t offset, size_t len);

size_t am_ioe_read(void *buf, size_t offset, size_t len) {
  ioe_read(offset, buf);
  return 0;
}

size_t am_ioe_write(const void *buf, size_t offset, size_t len) {
  ioe_write(offset, (void *)buf);
  return 0;
}

int fs_open(const char *pathname, int flags, int mode) {
  for (int i = 0; i < sizeof(file_table) / sizeof(Finfo); i++) {
    if (strcmp(pathname, file_table[i].name) == 0) {
      file_table[i].open_offset = 0;
      return i;
    }
  }
  panic("not found file %s", pathname);
  return -1;
}
size_t fs_read(int fd, void *buf, size_t len) {
  if(file_table[fd].read) { // avoid panic
    return file_table[fd].read(buf, file_table[fd].open_offset, len);
  }
  size_t curlen = (len < file_table[fd].size - file_table[fd].open_offset) ? len : file_table[fd].size - file_table[fd].open_offset;
  int ret = ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, curlen);
  file_table[fd].open_offset += curlen;
  return ret;
}
size_t fs_write(int fd, const void *buf, size_t len) {
  if(file_table[fd].write) { // avoid panic
    return file_table[fd].write(buf, file_table[fd].open_offset, len);
  }
  size_t curlen = (len < file_table[fd].size - file_table[fd].open_offset) ? len : file_table[fd].size - file_table[fd].open_offset;
  int ret = ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, curlen);
  file_table[fd].open_offset += curlen;
  return ret;
}
size_t fs_lseek(int fd, size_t offset, int whence) {
  switch(whence) {
    case SEEK_SET: {
      file_table[fd].open_offset = offset; break;
    }
    case SEEK_CUR: {
      file_table[fd].open_offset += offset; break;
    }
    case SEEK_END: {
      file_table[fd].open_offset = file_table[fd].size + offset; break;
    }
    default: panic("invalid whence %d", whence);
  }
  if(file_table[fd].open_offset > file_table[fd].size) {
    file_table[fd].open_offset = file_table[fd].size;
  }
  if(file_table[fd].open_offset < 0) {
    file_table[fd].open_offset = 0;
  }
  return file_table[fd].open_offset;
}
int fs_close(int fd) {
  file_table[fd].open_offset = 0;
  return 0;
}