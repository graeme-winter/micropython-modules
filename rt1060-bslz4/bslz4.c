#include "py/dynruntime.h"
#include <stdint.h>

STATIC mp_obj_t bslz4_init(mp_obj_t ny_obj, mp_obj_t nx_obj) {
  return mp_obj_new_int(0);
}

STATIC mp_obj_t bslz4_deinit(void) {
  return mp_obj_new_int(0);
}

STATIC mp_obj_t bslz4_block(mp_obj_t block_obj) {
  mp_buffer_info_t bufinfo;
  mp_get_buffer_raise(block_obj, &bufinfo, MP_BUFFER_RW);
  // mp_int_t nsignal = signal_filter_block((uint16_t *)bufinfo.buf);
  return mp_obj_new_int(0);
}

STATIC mp_obj_t bslz4_reset(void) {
  return mp_obj_new_int(0);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(bslz4_init_obj, bslz4_init);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(bslz4_deinit_obj, bslz4_deinit);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(bslz4_block_obj, bslz4_block);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(bslz4_reset_obj, bslz4_reset);

mp_obj_t mpy_init(mp_obj_fun_bc_t *self, size_t n_args, size_t n_kw,
                  mp_obj_t *args) {
  MP_DYNRUNTIME_INIT_ENTRY

  mp_store_global(MP_QSTR_init, MP_OBJ_FROM_PTR(&bslz4_init_obj));
  mp_store_global(MP_QSTR_deinit, MP_OBJ_FROM_PTR(&bslz4_deinit_obj));
  mp_store_global(MP_QSTR_block, MP_OBJ_FROM_PTR(&bslz4_block_obj));
  mp_store_global(MP_QSTR_reset, MP_OBJ_FROM_PTR(&bslz4_reset_obj));

  MP_DYNRUNTIME_INIT_EXIT
}
