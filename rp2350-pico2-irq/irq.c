#include "py/dynruntime.h"

volatile unsigned int original_handler;

#define VTOR_ADDR 0xe000ed08

__inline__ static void toggle(uint pin) {
  __asm("mcr p0, #5, %0, c0, c0" : : "r" (pin));
}

void irq(void) {
  // toggle GPIO 2
  // *(unsigned int *)0xd0000028 = 0x4;
  toggle(2);
  // clear IRQ in GPIO registers
  *(unsigned int *)0x40028230 = 0xc0;
}

static mp_obj_t irq_init(void) {
  if (original_handler == 0) {
    unsigned int *VTOR = *(unsigned int **)VTOR_ADDR;
    original_handler = VTOR[16 + 21];
    VTOR[16 + 21] = (unsigned int)&irq;
  }
  return mp_obj_new_int(0);
}

static mp_obj_t irq_deinit(void) {
  if (original_handler) {
    unsigned int *VTOR = *(unsigned int **)VTOR_ADDR;
    VTOR[16 + 21] = original_handler;
    original_handler = 0;
  }
  return mp_obj_new_int(0);
}

static MP_DEFINE_CONST_FUN_OBJ_0(irq_init_obj, irq_init);
static MP_DEFINE_CONST_FUN_OBJ_0(irq_deinit_obj, irq_deinit);

mp_obj_t mpy_init(mp_obj_fun_bc_t *self, size_t n_args, size_t n_kw,
                  mp_obj_t *args) {
  MP_DYNRUNTIME_INIT_ENTRY

  original_handler = 0;

  mp_store_global(MP_QSTR_init, MP_OBJ_FROM_PTR(&irq_init_obj));
  mp_store_global(MP_QSTR_deinit, MP_OBJ_FROM_PTR(&irq_deinit_obj));

  MP_DYNRUNTIME_INIT_EXIT
}
