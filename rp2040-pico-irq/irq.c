#include "py/dynruntime.h"

unsigned int original_handler;
unsigned int original_handler_set;

#define VTOR_ADDR 0xe000ed08

void irq(void) {
  // toggle GPIO 20
  *(unsigned int *) 0xd000001c = 0x1 << 20;
  // clear IRQ @ 19
  *(unsigned int *) 0x400140f8 = 0xf << 12;
}

STATIC mp_obj_t irq_init(void) {
  if (original_handler_set == 0) {
    unsigned int *VTOR = *(unsigned int **) VTOR_ADDR;
    original_handler = VTOR[16 + 13];
    VTOR[16 + 13] = (unsigned int) &irq;
    original_handler_set = 1;
  }
  return mp_obj_new_int(0);
}

STATIC mp_obj_t irq_deinit(void) {
  if (original_handler_set == 1) {
    unsigned int *VTOR = *(unsigned int **) VTOR_ADDR;
    VTOR[16 + 13] = original_handler;
    original_handler_set = 0;
  }
  return mp_obj_new_int(0);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(irq_init_obj, irq_init);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(irq_deinit_obj, irq_deinit);

mp_obj_t mpy_init(mp_obj_fun_bc_t *self, size_t n_args, size_t n_kw,
                  mp_obj_t *args) {
  MP_DYNRUNTIME_INIT_ENTRY

  original_handler_set = 0;

  mp_store_global(MP_QSTR_init, MP_OBJ_FROM_PTR(&irq_init_obj));
  mp_store_global(MP_QSTR_deinit, MP_OBJ_FROM_PTR(&irq_deinit_obj));

  MP_DYNRUNTIME_INIT_EXIT
}
