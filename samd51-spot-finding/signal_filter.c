#include <math.h>
#include "py/dynruntime.h"

// global variables - strictly some of these are not needed

uint32_t *im = NULL;
uint32_t *m_sat = NULL;
uint32_t *i_sat = NULL;
uint32_t *i2_sat = NULL;

uint32_t height = 0;
uint32_t width = 0;
uint32_t knl = 0;
uint32_t row = 0;

int signal_filter_init(uint32_t _height, uint32_t _width, uint32_t _knl) {
  if (im != NULL) {
    return 1;
  }
  height = _height;
  width = _width;
  knl = _knl;

  // buffer size is 2 * KNL + 1 rows, + 1 more row for the pre-subtraction row
  uint32_t nn = (2 * _knl + 2) * _width;

  im = (uint32_t *)m_malloc(sizeof(uint32_t) * nn);
  m_sat = (uint32_t *)m_malloc(sizeof(uint32_t) * nn);
  i_sat = (uint32_t *)m_malloc(sizeof(uint32_t) * nn);
  i2_sat = (uint32_t *)m_malloc(sizeof(uint32_t) * nn);

  return 0;
}

int signal_filter_deinit(void) {
  if (im == NULL) {
    return 1;
  }

  m_free(im);
  m_free(m_sat);
  m_free(i_sat);
  m_free(i2_sat);

  im = m_sat = i_sat = i2_sat = NULL;

  return 0;
}

int signal_filter_row(uint16_t *io_row) {
  float sigma_b = 6.0f, sigma_s = 3.0f;
  uint32_t knl2 = 2 * knl + 1;

  if (row < height) {
    // move rows up if we are past the start-up region
    if (row > knl2) {
      for (uint32_t i = 0; i < knl2; i++) {
        uint32_t off = i * width;
        for (uint32_t j = 0; j < width; j++) {
          im[off + j] = im[off + j + width];
          m_sat[off + j] = m_sat[off + j + width];
          i_sat[off + j] = i_sat[off + j + width];
          i2_sat[off + j] = i2_sat[off + j + width];
        }
      }
    }

    // populate row, update SAT
    uint32_t _m = 0, _i = 0, _i2 = 0;
    for (uint32_t j = 0; j < width; j++) {
      uint32_t m = io_row[j] > 0xfffd ? 0 : 1;
      uint32_t p = m * io_row[j];
      uint32_t i = row < knl2 ? row * width : knl2 * width;
      _m += m;
      _i += p;
      _i2 += p * p;
      im[i + j] = p;
      m_sat[i + j] = row > 0 ? _m + m_sat[i + j - width] : _m;
      i_sat[i + j] = row > 0 ? _i + i_sat[i + j - width] : _i;
      i2_sat[i + j] = row > 0 ? _i2 + i2_sat[i + j - width] : _i2;
    }
  }

  // at the very start we cannot do anything useful
  if (row < knl) {
    row++;
    return 0;
  }

  // after that we can start to compute signal tables for earlier rows
  for (uint32_t j = 0; j < width; j++) {

    int32_t j0 = j - knl - 1;
    int32_t j1 = j < (width - knl) ? j + knl : width - 1;

    int32_t i1 = row < knl2 ? row : knl2;
    int32_t i = row < height ? i1 - knl : i1 - (knl - (row - height)) + 1;
    int32_t i0 = i - knl - 1;

    int32_t a = i1 * width + j1;
    int32_t b = i0 * width + j1;
    int32_t c = i1 * width + j0;
    int32_t d = i0 * width + j0;

    uint32_t m_sum = m_sat[a], i_sum = i_sat[a], i2_sum = i2_sat[a];

    if (j0 >= 0 && i0 >= 0) {
      m_sum += m_sat[d] - m_sat[b] - m_sat[c];
      i_sum += i_sat[d] - i_sat[b] - i_sat[c];
      i2_sum += i2_sat[d] - i2_sat[b] - i2_sat[c];
    } else if (j0 >= 0) {
      m_sum -= m_sat[c];
      i_sum -= i_sat[c];
      i2_sum -= i2_sat[c];
    } else if (i0 >= 0) {
      m_sum -= m_sat[b];
      i_sum -= i_sat[b];
      i2_sum -= i2_sat[b];
    }

    // N.B. for photon counting detectors the threshold is usually zero,
    // masked pixels have value zero so cannot be signal - if the pixel is
    // > 0 then the sum must be

    uint16_t signal = 0;
    uint32_t p = im[i * width + j];

    if (p > 0 && m_sum >= 2) {
      float bg_lhs = (float)m_sum * i2_sum - (float)i_sum * i_sum -
                     (float)i_sum * (m_sum - 1);
      float bg_rhs = i_sum * sigma_b * (float) sqrtf((float)2 * (m_sum - 1));
      uint16_t background = bg_lhs > bg_rhs;
      float fg_lhs = (float) m_sum * p - (float)i_sum;
      float fg_rhs = sigma_s * (float) sqrtf((float)i_sum * m_sum);
      uint16_t foreground = fg_lhs > fg_rhs;
      signal = background && foreground;
    }
    io_row[j] = signal;
  }

  row++;

  if (row == height + knl)
    row = 0;

  return 0;
}
