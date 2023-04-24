#include "py/dynruntime.h"

// memory structure - interrupt vector is length > 128 words -> 256 word alignment
// needed so just allocate 2 kB of memory, seek to point where this is aligned
// correctly

mp_uint_t *buffer = NULL;
mp_uint_t *irq_vector_copy = NULL;

STATIC mp_obj_t drive_irq_init(void) {
  // allocate buffer of 2 x size to give space to align with 1024 kB boundary
  if (buffer == NULL) {
    buffer = m_malloc(2 * 1024);
    irq_vector_copy = (mp_uint_t *)(((mp_uint_t)buffer & (~1023)) + 1024);

    // FIXME copy vector from current source location
    // FIXME update VTOR
    // FIXME register additional handler
    // FIXME enable IRQ
  }

  mp_int_t result = (mp_int_t)irq_vector_copy;
  return mp_obj_new_int(result);
}

STATIC mp_obj_t drive_irq_deinit(void) {
  if (buffer) {
    // FIXME disable IRQ
    // FIXME revert VTOR

    m_free(buffer);
    buffer = NULL;
    irq_vector_copy = NULL;
  }

  mp_int_t result = (mp_int_t)irq_vector_copy;
  return mp_obj_new_int(result);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(drive_irq_init_obj, drive_irq_init);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(drive_irq_deinit_obj, drive_irq_deinit);

mp_obj_t mpy_init(mp_obj_fun_bc_t *self, size_t n_args, size_t n_kw,
                  mp_obj_t *args) {
  MP_DYNRUNTIME_INIT_ENTRY

  mp_store_global(MP_QSTR_init, MP_OBJ_FROM_PTR(&drive_irq_init_obj));
  mp_store_global(MP_QSTR_deinit, MP_OBJ_FROM_PTR(&drive_irq_deinit_obj));

  MP_DYNRUNTIME_INIT_EXIT
}
