/***************************************************************************************
 * Copyright (c) 2014-2022 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 *PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 *KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 *NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include <SDL2/SDL.h>
#include <common.h>
#include <device/map.h>

enum {
  reg_freq,
  reg_channels,
  reg_samples,
  reg_sbuf_size,
  reg_init,
  reg_count,
  nr_reg
};
// 每秒采样数, 通道数量：1单声道，2立体声,音频数据中每个样本所占的位数
// sbuf_size寄存器可读出流缓冲区的大小,init寄存器用于初始化,count寄存器可以读出当前流缓冲区已经使用的大小

static uint8_t *sbuf = NULL;
static uint32_t *audio_base = NULL;
static uint32_t sbuf_pos = 0;

// 如果回调函数需要的数据量大于当前流缓冲区中的数据量,
// 你还需要把SDL提供的缓冲区剩余的部分清零, 以避免把一些垃圾数据当做音频
void sdl_audio_callback(void *userdata, Uint8 *stream, int len) {
  SDL_LockAudio();
  SDL_memset(stream, 0, len);
  uint32_t remdata = audio_base[reg_count];
  uint32_t cpy_len = (len < remdata) ? len : remdata;
  uint32_t buf_size = audio_base[reg_sbuf_size];
  if (cpy_len + sbuf_pos < buf_size) {
    SDL_memcpy(stream, sbuf + sbuf_pos, cpy_len);
  } else {
    uint32_t len1 = buf_size - sbuf_pos;
    SDL_memcpy(stream, sbuf + sbuf_pos, len1);
    uint32_t len2 = cpy_len - len1;
    SDL_memcpy(stream + len1, sbuf, len2);
  }
  sbuf_pos = (sbuf_pos + cpy_len) % buf_size;
  audio_base[reg_count] -= cpy_len;
  SDL_UnlockAudio();
}

static void audio_io_handler(uint32_t offset, int len, bool is_write) {
  if (audio_base[reg_init] == 1 && is_write) {
    SDL_AudioSpec s = {};
    s.format = AUDIO_S16SYS;
    s.userdata = NULL;
    s.freq = audio_base[reg_freq];
    s.channels = audio_base[reg_channels];
    s.samples = audio_base[reg_samples];
    s.callback = sdl_audio_callback;
    audio_base[reg_count] = 0;
    audio_base[reg_sbuf_size] = CONFIG_SB_SIZE;
    int ret = SDL_InitSubSystem(SDL_INIT_AUDIO);
    if (ret == 0) {
      SDL_OpenAudio(&s, NULL);
      SDL_PauseAudio(0);
    }
    audio_base[reg_init] = 0;
  }
}

void init_audio() {
  uint32_t space_size = sizeof(uint32_t) * nr_reg;
  audio_base = (uint32_t *)new_space(space_size);
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map("audio", CONFIG_AUDIO_CTL_PORT, audio_base, space_size,
              audio_io_handler);
#else
  add_mmio_map("audio", CONFIG_AUDIO_CTL_MMIO, audio_base, space_size,
               audio_io_handler);
#endif

  sbuf = (uint8_t *)new_space(CONFIG_SB_SIZE);
  add_mmio_map("audio-sbuf", CONFIG_SB_ADDR, sbuf, CONFIG_SB_SIZE, NULL);
}
