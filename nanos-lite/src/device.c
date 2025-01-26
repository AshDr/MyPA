#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
  for(size_t i = 0; i < len; i++) {
    putch(((char *)buf)[i]);
  }
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  if(ev.keycode == AM_KEY_NONE) {
    return 0;
  }
  return snprintf((char *)buf, len, "%s %s\n", ev.keydown? "kd" : "ku", keyname[ev.keycode]);
}
static uint32_t scree_w = 0, screen_h = 0;
size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  AM_GPU_CONFIG_T cfg = io_read(AM_GPU_CONFIG);
  scree_w = cfg.width;
  screen_h = cfg.height;
  printf("scree_w = %d, screen_h = %d\n", scree_w, screen_h);
  return snprintf((char *)buf, len, "WIDTH: %d\nHEIGHT: %d\n", cfg.width, cfg.height);
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  io_write(AM_GPU_FBDRAW,(offset % (scree_w * 4)) / 4, offset / (scree_w * 4), (uint32_t *)buf, len / 4, 1, true);
  return 1;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
