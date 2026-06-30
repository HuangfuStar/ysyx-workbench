#ifndef __CPU_IRINGBUF_H__
#define __CPU_IRINGBUF_H__

#include <common.h>

void iringbuf_add(vaddr_t pc, uint32_t inst, uint8_t ilen);
void iringbuf_trace_msg(void);

#endif
