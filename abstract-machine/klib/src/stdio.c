#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  char buf[32768]; // KISS
  va_list ap;

  va_start(ap, fmt);
  int len = vsprintf(buf, fmt, ap);
  va_end(ap);

  for (int i = 0; i < len; i++) {
    putch(buf[i]);
  }

  return len;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  int len = 0;

  for (const char *p = fmt; *p; p++) {
    if (*p != '%') {
      out[len++] = *p;
      continue;
    }

    p++;
    int zero_pad = 0;
    int width = 0;
    if (*p == '0') {
      zero_pad = 1;
      p++;
      while (*p >= '0' && *p <= '9') {
        width = width * 10 + (*p - '0');
        p++;
      }
    }

    if (*p == 'd') {
      int val = va_arg(ap, int);
      unsigned int mag;
      char buf[16];
      int n = 0;

      if (val < 0) {
        out[len++] = '-';
        mag = 0u - (unsigned int)val;
        if (width > 0) width--;
      } else {
        mag = (unsigned int)val;
      }

      do {
        buf[n++] = '0' + (mag % 10);
        mag /= 10;
      } while (mag != 0);

      while (zero_pad && n < width) {
        out[len++] = '0';
        width--;
      }

      while (n > 0) {
        out[len++] = buf[--n];
      }
    } else if (*p == 'x') {
      unsigned int val = va_arg(ap, unsigned int);
      char buf[16];
      int n = 0;

      do {
        unsigned int digit = val & 0xf;
        buf[n++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
        val >>= 4;
      } while (val != 0);

      while (zero_pad && n < width) {
        out[len++] = '0';
        width--;
      }

      while (n > 0) {
        out[len++] = buf[--n];
      }
    } else if (*p == 'c') {
      out[len++] = (char)va_arg(ap, int);
    } else if (*p == 's') {
      const char *s = va_arg(ap, const char *);
      if (s == NULL) s = "(null)";
      while (*s) {
        out[len++] = *s++;
      }
    } else if (*p == '%') {
      out[len++] = '%';
    } else {
      assert(0);
    }
  }

  out[len] = '\0';
  return len;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int len = vsprintf(out, fmt, ap);
  va_end(ap);
  return len;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int len = vsnprintf(out, n, fmt, ap);
  va_end(ap);
  return len;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
