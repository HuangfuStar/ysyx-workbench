#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t idx = 0;
  while (s[idx]) idx++;
  return idx;
}

char *strcpy(char *dst, const char *src) {
  size_t i;

  for (i = 0; src[i]; i++) dst[i] = src[i];
  dst[i] = '\0';
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  panic("Not implemented");
}

char *strcat(char *dst, const char *src) {
  size_t dst_len = strlen(dst);
  size_t i = 0;
  for (; src[i]; i++) dst[dst_len + i] = src[i];
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  int idx = 0;
  while (s1[idx] && s2[idx] && s1[idx] == s2[idx]) idx++;
  if (!s1[idx] && !s2[idx]) return 0;
  else return s1[idx] - s2[idx];
}

int strncmp(const char *s1, const char *s2, size_t n) {
  for (size_t i = 0; i < n; i++) {
    unsigned char c1 = (unsigned char)s1[i];
    unsigned char c2 = (unsigned char)s2[i];
    if (c1 != c2) return c1 - c2;
    if (c1 == '\0') return 0;
  }
  return 0;
}

void *memset(void *s, int c, size_t n) {
  for (size_t i = 0; i < n; i++) ((unsigned char *)s)[i] = c;
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  unsigned char *d = (unsigned char *)dst;
  const unsigned char *s = (const unsigned char *)src;

  if (d == s || n == 0) return dst;

  if (d < s) {
    for (size_t i = 0; i < n; i++) d[i] = s[i];
  } else {
    for (size_t i = n; i > 0; i--) d[i - 1] = s[i - 1];
  }

  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  unsigned char *dst = (unsigned char *)out;
  const unsigned char *src = (const unsigned char *)in;

  for (size_t i = 0; i < n; i++) {
    dst[i] = src[i];
  }

  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  if (n == 0) return 0;
  size_t i;
  for (i = 0; i < n && ((unsigned char *) s1)[i] == ((unsigned char *) s2)[i]; i++);
  if (i == n) return 0;
  return (((unsigned char *) s1)[i]) - (((unsigned char *) s2)[i]);
}

#endif
