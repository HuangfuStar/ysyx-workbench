#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

#define IRQ_SYSCALL 11

static Context* (*user_handler)(Event, Context*) = NULL;

Context* __am_irq_handle(Context *c) {
  assert(c != NULL);
  if (!user_handler) return c;

  Event ev = {0};
  switch (c->mcause) {
    case IRQ_SYSCALL:
      c->mepc += 4;
      ev.event = (c->GPR1 == (uintptr_t)-1 ? EVENT_YIELD : EVENT_SYSCALL);
      break;
    default:
      ev.event = EVENT_ERROR;
      break;
  }

  c = user_handler(ev, c);
  assert(c != NULL);
  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  Context *c = (Context *)kstack.end - 1;
  *c = (Context) { 0 };

  c->mstatus = 0x1800;
  c->mepc = (uintptr_t)entry;
  c->GPR2 = (uintptr_t)arg;

  return c;
}

void yield() {
#ifdef __riscv_e
  asm volatile("li a5, -1; ecall");
#else
  asm volatile("li a7, -1; ecall");
#endif
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
