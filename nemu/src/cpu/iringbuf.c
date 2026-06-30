#include <cpu/iringbuf.h>
#include <stdio.h>

typedef struct {
  vaddr_t pc;
  uint32_t inst;
  uint8_t ilen;
  bool valid;
} iringbuf_entry_t;

typedef struct {
  iringbuf_entry_t buf[CONFIG_IRINGBUF_NR];
  int head;
  int count;
} iringbuf_t;

static iringbuf_t iringbuf_queue = {};

void iringbuf_add(vaddr_t pc, uint32_t inst, uint8_t ilen) {
  iringbuf_entry_t *entry = &iringbuf_queue.buf[iringbuf_queue.head];
  entry->pc = pc;
  entry->inst = inst;
  entry->ilen = ilen;
  entry->valid = true;

  iringbuf_queue.head = (iringbuf_queue.head + 1) % CONFIG_IRINGBUF_NR;
  if (iringbuf_queue.count < CONFIG_IRINGBUF_NR) {
    iringbuf_queue.count++;
  }
}

void iringbuf_trace_msg(void) {
  if (iringbuf_queue.count == 0) {
    printf("Instruction ring buffer is empty.\n");
    return;
  }

  void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);

  printf("Instruction ring buffer:\n");

  int start = (iringbuf_queue.head - iringbuf_queue.count + CONFIG_IRINGBUF_NR) % CONFIG_IRINGBUF_NR;
  for (int i = 0; i < iringbuf_queue.count; i++) {
    int idx = (start + i) % CONFIG_IRINGBUF_NR;
    iringbuf_entry_t *entry = &iringbuf_queue.buf[idx];
    if (!entry->valid) {
      continue;
    }

    uint8_t code[8] = {};
    for (int j = 0; j < entry->ilen; j++) {
      code[j] = (entry->inst >> (j * 8)) & 0xff;
    }

    char asm_buf[128] = {};
    disassemble(asm_buf, sizeof(asm_buf), entry->pc, code, entry->ilen);
    printf("%c " FMT_WORD ": %s\n",
        (i == iringbuf_queue.count - 1) ? '>' : ' ',
        entry->pc, asm_buf);
  }
}
