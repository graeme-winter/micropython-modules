#include "py/dynruntime.h"

STATIC mp_obj_t drive_irq(mp_obj_t obj0, mp_obj_t obj1) {
  mp_int_t arg0 = mp_obj_get_int(obj0);
  mp_int_t arg1 = mp_obj_get_int(obj1);
  mp_int_t result = arg0 + arg1;
  return mp_obj_new_int(result);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(drive_irq_obj, drive_irq);

mp_obj_t mpy_init(mp_obj_fun_bc_t *self, size_t n_args, size_t n_kw,
                  mp_obj_t *args) {
  MP_DYNRUNTIME_INIT_ENTRY

  mp_store_global(MP_QSTR_drive_irq, MP_OBJ_FROM_PTR(&drive_irq_obj));

  MP_DYNRUNTIME_INIT_EXIT
}

