#include "py/dynruntime.h"

unsigned int original_handler;
unsigned int original_handler_set;
volatile unsigned int systick;

#define VTOR_ADDR 0xe000ed08
#define SYST_CVR 0xe000e018

void irq(void) {
  // grab the time delta
  systick = *(unsigned int *)SYST_CVR;

  // toggle GPIO 25 as a sign of life
  *(unsigned int *)0xd000001c = 0x1 << 25;
}

STATIC mp_obj_t irq_init(void) {
  if (original_handler_set == 0) {
    unsigned int *VTOR = *(unsigned int **)VTOR_ADDR;
    original_handler = VTOR[0xf];
    VTOR[0xf] = (unsigned int)&irq;
    original_handler_set = 1;
  }
  return mp_obj_new_int(0);
}

STATIC mp_obj_t irq_deinit(void) {
  if (original_handler_set == 1) {
    unsigned int *VTOR = *(unsigned int **)VTOR_ADDR;
    VTOR[0xf] = original_handler;
    original_handler_set = 0;
  }
  return mp_obj_new_int(systick);
}

STATIC mp_obj_t irq_get(void) {
  return mp_obj_new_int(systick);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(irq_init_obj, irq_init);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(irq_deinit_obj, irq_deinit);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(irq_get_obj, irq_get);

mp_obj_t mpy_init(mp_obj_fun_bc_t *self, size_t n_args, size_t n_kw,
                  mp_obj_t *args) {
  MP_DYNRUNTIME_INIT_ENTRY

  original_handler_set = 0;

  mp_store_global(MP_QSTR_init, MP_OBJ_FROM_PTR(&irq_init_obj));
  mp_store_global(MP_QSTR_deinit, MP_OBJ_FROM_PTR(&irq_deinit_obj));
  mp_store_global(MP_QSTR_get, MP_OBJ_FROM_PTR(&irq_get_obj));

  MP_DYNRUNTIME_INIT_EXIT
}
