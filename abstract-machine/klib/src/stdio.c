#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

typedef struct {
  char *buf;
  size_t size;
  int len;
} out_ctx_t;

static void out_char(out_ctx_t *ctx, char ch) {
  if (ctx->size > 0 && (size_t)ctx->len + 1 < ctx->size) {
    ctx->buf[ctx->len] = ch;
  }
  ctx->len++;
}

static void out_pad(out_ctx_t *ctx, char ch, int count) {
  while (count-- > 0) {
    out_char(ctx, ch);
  }
}

static void out_str(out_ctx_t *ctx, const char *s) {
  while (*s) {
    out_char(ctx, *s++);
  }
}

static void out_uint(out_ctx_t *ctx, unsigned long val, unsigned base, int width, char pad, bool upper) {
  char buf[sizeof(unsigned long) * 8];
  int n = 0;

  do {
    unsigned digit = val % base;
    buf[n++] = (digit < 10) ? ('0' + digit) : ((upper ? 'A' : 'a') + digit - 10);
    val /= base;
  } while (val != 0);

  out_pad(ctx, pad, width - n);
  while (n > 0) {
    out_char(ctx, buf[--n]);
  }
}

static void out_int(out_ctx_t *ctx, long val, int width, char pad) {
  unsigned long mag;

  if (val < 0) {
    out_char(ctx, '-');
    mag = 0ul - (unsigned long)val;
    width--;
  } else {
    mag = (unsigned long)val;
  }

  out_uint(ctx, mag, 10, width, pad, false);
}

static int format_to_buffer(char *out, size_t size, const char *fmt, va_list ap) {
  out_ctx_t ctx = { .buf = out, .size = size, .len = 0 };

  for (const char *p = fmt; *p; p++) {
    if (*p != '%') {
      out_char(&ctx, *p);
      continue;
    }

    p++;
    int width = 0;
    char pad = ' ';
    bool long_arg = false;

    if (*p == '0') {
      pad = '0';
    }
    while (*p >= '0' && *p <= '9') {
      width = width * 10 + (*p - '0');
      p++;
    }

    if (*p == 'l') {
      long_arg = true;
      p++;
    }

    switch (*p) {
      case 'd':
        if (long_arg) out_int(&ctx, va_arg(ap, long), width, pad);
        else out_int(&ctx, va_arg(ap, int), width, pad);
        break;
      case 'u':
        if (long_arg) out_uint(&ctx, va_arg(ap, unsigned long), 10, width, pad, false);
        else out_uint(&ctx, va_arg(ap, unsigned int), 10, width, pad, false);
        break;
      case 'x':
        if (long_arg) out_uint(&ctx, va_arg(ap, unsigned long), 16, width, pad, false);
        else out_uint(&ctx, va_arg(ap, unsigned int), 16, width, pad, false);
        break;
      case 'X':
        if (long_arg) out_uint(&ctx, va_arg(ap, unsigned long), 16, width, pad, true);
        else out_uint(&ctx, va_arg(ap, unsigned int), 16, width, pad, true);
        break;
      case 'c':
        out_char(&ctx, (char)va_arg(ap, int));
        break;
      case 's': {
        const char *s = va_arg(ap, const char *);
        out_str(&ctx, s == NULL ? "(null)" : s);
        break;
      }
      case '%':
        out_char(&ctx, '%');
        break;
      default:
        panic(fmt);
    }
  }

  if (ctx.size > 0) {
    size_t end = ((size_t)ctx.len < ctx.size) ? (size_t)ctx.len : (ctx.size - 1);
    ctx.buf[end] = '\0';
  }

  return ctx.len;
}

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
  return format_to_buffer(out, (size_t)-1, fmt, ap);
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
  return format_to_buffer(out, n, fmt, ap);
}

#endif
