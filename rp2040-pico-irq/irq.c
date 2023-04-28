#include "py/dynruntime.h"

unsigned int original_handler;

void irq(void) {
}

STATIC mp_obj_t irq_init(void) {
  return mp_obj_new_int(0);
}

STATIC mp_obj_t irq_deinit(void) {
  return mp_obj_new_int(0);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(irq_init_obj, irq_init);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(irq_deinit_obj, irq_deinit);

mp_obj_t mpy_init(mp_obj_fun_bc_t *self, size_t n_args, size_t n_kw,
                  mp_obj_t *args) {
  MP_DYNRUNTIME_INIT_ENTRY

  mp_store_global(MP_QSTR_init, MP_OBJ_FROM_PTR(&irq_init_obj));
  mp_store_global(MP_QSTR_deinit, MP_OBJ_FROM_PTR(&irq_deinit_obj));

  MP_DYNRUNTIME_INIT_EXIT
}
