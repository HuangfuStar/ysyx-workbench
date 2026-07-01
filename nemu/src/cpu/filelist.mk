ifeq ($(CONFIG_IRINGBUF),)
SRCS-BLACKLIST-y += src/cpu/iringbuf.c
endif

ifeq ($(CONFIG_FTRACE),)
SRCS-BLACKLIST-y += src/cpu/ftrace.c
endif
