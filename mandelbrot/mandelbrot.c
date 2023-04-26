#include "py/dynruntime.h"

extern int mandelrow_impl(int addr, int Ci);

STATIC mp_obj_t mandelrow(mp_obj_t addr_obj, mp_obj_t ci_obj) {
  mp_int_t addr = mp_obj_get_int(addr_obj);
  mp_int_t ci = mp_obj_get_int(ci_obj);
  mp_int_t result = mandelrow_impl(addr, ci);
  return mp_obj_new_int(result);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(mandelrow_obj, mandelrow);

mp_obj_t mpy_init(mp_obj_fun_bc_t *self, size_t n_args, size_t n_kw,
                  mp_obj_t *args) {
  MP_DYNRUNTIME_INIT_ENTRY

  mp_store_global(MP_QSTR_mandelbrot, MP_OBJ_FROM_PTR(&mandelrow_obj));

  MP_DYNRUNTIME_INIT_EXIT
}
