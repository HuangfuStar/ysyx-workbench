#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

static int screen_w = 0;
static int screen_h = 0;

void __am_gpu_init() {
  uint32_t size = inl(VGACTL_ADDR);
  screen_w = size >> 16;
  screen_h = size & 0xffff;
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = screen_w, .height = screen_h,
    .vmemsz = 0
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  uint32_t *pixels = (uint32_t *)ctl->pixels;

  if (pixels != NULL) {
    uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
    int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;

    panic_on(x < 0 || y < 0 || w < 0 || h < 0, "bad draw command");
    panic_on(x + w > screen_w || y + h > screen_h, "bad draw command");

    for (int j = 0; j < h; j ++) {
      for (int i = 0; i < w; i ++) {
        fb[(y + j) * screen_w + (x + i)] = pixels[j * w + i];
      }
    }
  }

  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
