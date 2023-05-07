#include <stdint.h>
#include "py/dynruntime.h"

int signal_filter_init(uint32_t height, uint32_t width, uint32_t knl);
int signal_filter_row(uint16_t *row);
int signal_filter_deinit(void);

STATIC mp_obj_t spot_finder_init(mp_obj_t ny_obj, mp_obj_t nx_obj) {
  mp_int_t ny = mp_obj_get_int(ny_obj);
  mp_int_t nx = mp_obj_get_int(nx_obj);
  uint32_t knl = 3;
  signal_filter_init((uint32_t) ny, (uint32_t) nx, knl);
  return mp_obj_new_int(0);
}

STATIC mp_obj_t spot_finder_deinit(void) {
  signal_filter_deinit();
  return mp_obj_new_int(0);
}

STATIC mp_obj_t spot_finder_row(mp_obj_t row_obj) {
  mp_buffer_info_t bufinfo;
  mp_get_buffer_raise(row_obj, &bufinfo, MP_BUFFER_RW);
  signal_filter_row((uint16_t *) bufinfo.buf);
  return mp_obj_new_int(0);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(spot_finder_init_obj, spot_finder_init);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(spot_finder_deinit_obj, spot_finder_deinit);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(spot_finder_row_obj, spot_finder_row);

mp_obj_t mpy_init(mp_obj_fun_bc_t *self, size_t n_args, size_t n_kw,
                  mp_obj_t *args) {
  MP_DYNRUNTIME_INIT_ENTRY

  mp_store_global(MP_QSTR_init, MP_OBJ_FROM_PTR(&spot_finder_init_obj));
  mp_store_global(MP_QSTR_deinit, MP_OBJ_FROM_PTR(&spot_finder_deinit_obj));
  mp_store_global(MP_QSTR_row, MP_OBJ_FROM_PTR(&spot_finder_row_obj));

  MP_DYNRUNTIME_INIT_EXIT
}
