#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <ncurses.h>
#include <term.h>
#include <pthread.h>

char ch_map[4] = {' ','-','=','$'};
// char ch_map[4] = {' ','$','$','$'};
int msleep(long msec);
void* frame_delay(void* arg);
void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
void setup_audio(ma_device* device, ma_decoder* decoder, const char* file);
void destroy_audio(ma_device* device, ma_decoder* decoder);

int main(int argc, char* argv[]) {
  int t_rows, t_cols;
  int f_width, f_height, f_chann;

  ma_decoder decoder;
  ma_device device;
  setup_audio(&device, &decoder, "res/bad_apple.wav");

  initscr(); {
    curs_set(0); // No cursor

    /* Calc viewport dims */
    // TODO: Fix y_len, x_len, y_0 calc
    getmaxyx(stdscr, t_rows, t_cols);
    const int y_len = 45; //const int y_len = t_rows;
    const int x_len = 100; //const int x_len = (y_len / 0.75f);
    const int y_0 = 1;
    const int x_0 = (t_cols - x_len) / 2.0f;

    /* Calc pixel offsets */
    stbi_info("res/frames/0001.png", &f_width, &f_height, &f_chann);
    const int dx = ceilf((float)f_width/(float)x_len);
    const int dy = ceilf((float)f_height/(float)y_len);

    /* Print loop */
    const int frames = 6572;
    for (int f = 1; f <= frames; ++f) {
      /* Delay thread */
      pthread_t th_delay;
      pthread_create(&th_delay, NULL, &frame_delay, NULL);
      
      /* Load frame */
      char file[20];
      sprintf(file, "res/frames/%04d.png", f);
      unsigned char* frame = stbi_load(file, &f_width, &f_height, &f_chann, 1);
      unsigned char* curr = frame;
      char data[f_height][f_width];
      for (int i = 0; i < f_height; ++i)
        for (int j = 0; j < f_width; ++j)
          data[i][j] = ch_map[(*curr++ >> 6)];
      stbi_image_free(frame);

      /* Map pixels */
      // TODO: Fix segfault with some offsets
      for (int y = 0; (y < y_len && y*dx < f_height); y++) {
        for (int x = 0; (x < x_len && x*dx < f_width); x++) {
          mvprintw(y+y_0, x+x_0, "%c", data[y*dy][x*dx]);
          //printf("\033[%d;%dH%s", y+y_0, x+x_0, data[y*dy][x*dx]);
        }
      }

      pthread_join(th_delay, NULL); // Wait for thread
      refresh();
    }
  } endwin();

  destroy_audio(&device, &decoder);
  return 0;
}
void* frame_delay(void* arg) {
  // TODO: Find a way to calculate delay for cpu
  int res;
  struct timespec ts;
  ts.tv_sec = 0;
  // ts.tv_nsec = 32660000; // desktop
  // ts.tv_nsec = 32350000; // laptop
  ts.tv_nsec = 32680000; // desktop

  do {
    res = nanosleep(&ts, &ts);
  } while(res);
  return arg;
}

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
    if (pDecoder == NULL) {
        return;
    }

    ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);

    (void)pInput;
}

void setup_audio(ma_device* device, ma_decoder* decoder, const char* file) {
  ma_result result;
  ma_device_config deviceConfig;

  result = ma_decoder_init_file(file, NULL, decoder);
  if (result != MA_SUCCESS) {
    exit(1);
  }

  deviceConfig = ma_device_config_init(ma_device_type_playback);
  deviceConfig.playback.format = decoder->outputFormat;
  deviceConfig.playback.channels = decoder->outputChannels;
  deviceConfig.sampleRate = decoder->outputSampleRate;
  deviceConfig.dataCallback = data_callback;
  deviceConfig.pUserData = decoder;

  if (ma_device_init(NULL, &deviceConfig, device) != MA_SUCCESS) {
    ma_decoder_uninit(decoder);
    exit(2);
  }

  if (ma_device_start(device) != MA_SUCCESS) {
    ma_device_uninit(device);
    ma_decoder_uninit(decoder);
    exit(3);
  }
}

void destroy_audio(ma_device* device, ma_decoder* decoder) {
  ma_device_uninit(device);
  ma_decoder_uninit(decoder);
}
