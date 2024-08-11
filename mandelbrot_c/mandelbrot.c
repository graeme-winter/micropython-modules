#include "py/dynruntime.h"

#define mul(a, b) (int32_t)((((int64_t)a) * ((int64_t)b)) >> 24)

int mandelrow_impl(int addr, int ci) {
  int32_t offset = 0;
  int *iter = (int *) addr;
  for (int32_t cr = -((2 << 24) - 0x4000); cr < 0x800000; cr += 0x8000) {
    int32_t count = 0;
    int32_t zr = 0, zi = 0;
    int32_t zr2, zi2;

    while (count != 0x1000) {
      zr2 = mul(zr, zr);
      zi2 = mul(zi, zi);
      if ((zr2 + zi2) >= 0x4000000)
        break;
      count++;
      zi = 2 * mul(zr, zi) + ci;
      zr = zr2 - zi2 + cr;
    }
    iter[offset] = count;
    offset++;
  } 

  return 0;
}

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
