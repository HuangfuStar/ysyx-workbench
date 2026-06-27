#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  int len = 0;

  va_start(ap, fmt);
  for (const char *p = fmt; *p; p++) {
    if (*p != '%') {
      out[len++] = *p;
      continue;
    }

    p++;
    if (*p == 'd') {
      int val = va_arg(ap, int);
      unsigned int mag;
      char buf[16];
      int n = 0;

      if (val < 0) {
        out[len++] = '-';
        mag = 0u - (unsigned int)val;
      } else {
        mag = (unsigned int)val;
      }

      do {
        buf[n++] = '0' + (mag % 10);
        mag /= 10;
      } while (mag != 0);

      while (n > 0) {
        out[len++] = buf[--n];
      }
    } else if (*p == 's') {
      const char *s = va_arg(ap, const char *);
      if (s == NULL) s = "(null)";
      while (*s) {
        out[len++] = *s++;
      }
    } else if (*p == '%') {
      out[len++] = '%';
    } else {
      out[len++] = '%';
      out[len++] = *p;
    }
  }
  va_end(ap);

  out[len] = '\0';
  return len;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
