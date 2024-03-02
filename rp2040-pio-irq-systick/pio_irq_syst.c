#include "py/dynruntime.h"

unsigned int original_handler;
unsigned int original_handler_set;

unsigned int *systick_readings;
volatile unsigned int systick_max;
volatile unsigned int irq_count;

#define VTOR_ADDR 0xe000ed08
#define VTOR_IRQ_OFFSET 16
#define PIO_IRQ_OFFSET 7

void pio_irq(void) {
  // record current systick value
  systick_readings[irq_count] = *(unsigned int *)0xe000e018;

  // clear pio_irq0 in PIO0 register §3.7
  *(unsigned int *)0x50200030 = 0x1;

  // increment count - rolling if we go over
  irq_count++;
  if (irq_count == systick_max)
    irq_count = 0;
}

STATIC mp_obj_t pio_irq_init(mp_obj_t addr_obj, mp_obj_t count_obj) {
  systick_readings = (unsigned int *)mp_obj_get_int(addr_obj);
  systick_max = (unsigned int)mp_obj_get_int(count_obj);
  irq_count = 0;
  if (original_handler_set == 0) {
    unsigned int *VTOR = *(unsigned int **)VTOR_ADDR;
    original_handler = VTOR[VTOR_IRQ_OFFSET + PIO_IRQ_OFFSET];
    VTOR[VTOR_IRQ_OFFSET + PIO_IRQ_OFFSET] = (unsigned int)&pio_irq;
    original_handler_set = 1;
  }
  return mp_obj_new_int(0);
}

STATIC mp_obj_t pio_irq_deinit(void) {
  if (original_handler_set == 1) {
    unsigned int *VTOR = *(unsigned int **)VTOR_ADDR;
    VTOR[VTOR_IRQ_OFFSET + PIO_IRQ_OFFSET] = original_handler;
    original_handler_set = 0;
  }
  return mp_obj_new_int(irq_count);
}

STATIC mp_obj_t pio_irq_get(mp_obj_t count_obj) {
  mp_int_t index = (unsigned int)mp_obj_get_int(count_obj);
  return mp_obj_new_int(systick_readings[index]);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(pio_irq_init_obj, pio_irq_init);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(pio_irq_deinit_obj, pio_irq_deinit);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pio_irq_get_obj, pio_irq_get);

mp_obj_t mpy_init(mp_obj_fun_bc_t *self, size_t n_args, size_t n_kw,
                  mp_obj_t *args) {
  MP_DYNRUNTIME_INIT_ENTRY

  original_handler_set = 0;

  mp_store_global(MP_QSTR_init, MP_OBJ_FROM_PTR(&pio_irq_init_obj));
  mp_store_global(MP_QSTR_deinit, MP_OBJ_FROM_PTR(&pio_irq_deinit_obj));
  mp_store_global(MP_QSTR_get, MP_OBJ_FROM_PTR(&pio_irq_get_obj));

  MP_DYNRUNTIME_INIT_EXIT
}
