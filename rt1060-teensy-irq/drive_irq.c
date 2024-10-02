#include "py/dynruntime.h"

// defines
#define GPIO2_BASE 0x401bc000
#define VTOR_ADDR 0xe000ed08

// memory structure - interrupt vector is length > 128 words -> 256 word
// alignment needed so just allocate 2 kB of memory, seek to point where this is
// aligned correctly

unsigned int *buffer = NULL;
unsigned int *irq_vector_copy = NULL;
unsigned int *VTOR_INIT = 0;

void irq_action(void) {
  // toggle D12 / B0_01
  *((unsigned int *)(GPIO2_BASE | 0x8c)) = 0x1 << 1;

  // clear IRQ on D10 / B0_00
  *(unsigned int *)(GPIO2_BASE | 0x18) = 0x1 << 0;
}

static mp_obj_t drive_irq_init(void) {
  // allocate buffer of 2 x size to give space to align with 1024 kB boundary
  if (buffer == NULL) {
    buffer = m_malloc(2 * 1024);
    irq_vector_copy = (unsigned int *)(((unsigned int)buffer & (~1023)) + 1024);

    VTOR_INIT = *(unsigned int **)VTOR_ADDR;

    // Copy vector from current source location
    for (int j = 0; j < (16 + 160); j++) {
      irq_vector_copy[j] = VTOR_INIT[j];
    }

    // Register additional handler - for GPIO1 / 0 has unshared IRQ
    irq_vector_copy[16 + 82] = (unsigned int)&irq_action;

    // Update VTOR
    *(unsigned int **)VTOR_ADDR = irq_vector_copy;
  }

  mp_int_t result = (mp_int_t)VTOR_INIT;
  return mp_obj_new_int(result);
}

static mp_obj_t drive_irq_deinit(void) {
  if (buffer) {
    // Revert VTOR
    *(unsigned int **)VTOR_ADDR = VTOR_INIT;

    m_free(buffer);
    buffer = NULL;
    irq_vector_copy = NULL;
  }

  mp_int_t result = (mp_int_t)irq_vector_copy;
  return mp_obj_new_int(result);
}

static MP_DEFINE_CONST_FUN_OBJ_0(drive_irq_init_obj, drive_irq_init);
static MP_DEFINE_CONST_FUN_OBJ_0(drive_irq_deinit_obj, drive_irq_deinit);

mp_obj_t mpy_init(mp_obj_fun_bc_t *self, size_t n_args, size_t n_kw,
                  mp_obj_t *args) {
  MP_DYNRUNTIME_INIT_ENTRY

  mp_store_global(MP_QSTR_init, MP_OBJ_FROM_PTR(&drive_irq_init_obj));
  mp_store_global(MP_QSTR_deinit, MP_OBJ_FROM_PTR(&drive_irq_deinit_obj));

  MP_DYNRUNTIME_INIT_EXIT
}
