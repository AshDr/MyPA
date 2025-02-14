#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() { // test code

  // int i;
  // uint32_t res_data = inl(VGACTL_ADDR);
  // int w = res_data >> 16;
  // int h = res_data & 0xffff; // TODO: get the correct height
  // uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  // for (i = 0; i < w * h; i++)
  //   fb[i] = i;
  // // init vmem data, single pixel is 4 bytes
  // outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  uint32_t data = inl(VGACTL_ADDR);
  uint32_t width = data >> 16;
  uint32_t height = data & 0xffff;
  *cfg = (AM_GPU_CONFIG_T){.present = true,
                           .has_accel = false,
                           .width = width,
                           .height = height,
                           .vmemsz = width*height*4};
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
  if (!ctl->sync && (w == 0 || h == 0))
    return;
  uint32_t *pixels = ctl->pixels;
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  uint32_t screen_w = inl(VGACTL_ADDR) >> 16;
  for (int i = y; i < y + h; i++)
    for (int j = x; j < x + w; j++)
      fb[i * screen_w + j] = pixels[w * (i - y) + (j - x)];
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) { status->ready = true; }
