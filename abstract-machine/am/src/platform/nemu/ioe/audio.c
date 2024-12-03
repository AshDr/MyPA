#include <am.h>
#include <nemu.h>

#define AUDIO_FREQ_ADDR (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR (AUDIO_ADDR + 0x14)

// static uint32_t sbuf_pos = 0;

void __am_audio_init() {}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
  // uint32_t buf_size = inl(AUDIO_SBUF_SIZE_ADDR);
  // cfg->present = true;
  // cfg->bufsize = buf_size;
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
  // outl(AUDIO_FREQ_ADDR, ctrl->freq);
  // outl(AUDIO_CHANNELS_ADDR, ctrl->channels);
  // outl(AUDIO_SAMPLES_ADDR, ctrl->samples);
  // outl(AUDIO_INIT_ADDR, 1); // 初始化声卡
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
  // stat->count = inl(AUDIO_COUNT_ADDR);
}

void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
  // 若当前流缓冲区的空闲空间少于即将写入的音频数据, 此次写入将会一直等待,
  // 直到有足够的空闲空间将音频数据完全写入流缓冲区才会返回
  // Area buf = ctl->buf;
  // uint32_t count = inl(AUDIO_COUNT_ADDR);
  // uint32_t buf_size = inl(AUDIO_SBUF_SIZE_ADDR);
  // uint8_t *sbuf = (uint8_t *)(uintptr_t)AUDIO_SBUF_ADDR;
  // for (int i = 0; i < buf.end - buf.start; i++) {
  //   sbuf[sbuf_pos] = ((uint8_t *)buf.start)[i];
  //   sbuf_pos = (sbuf_pos + 1) % buf_size;
  // }
  // outl(AUDIO_COUNT_ADDR, count + buf.end - buf.start);
}
