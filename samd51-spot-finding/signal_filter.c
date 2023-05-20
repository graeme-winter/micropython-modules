#include "py/dynruntime.h"
#include <math.h>
#include <stdint.h>

/* Spot structure:
 *
 * sum of intensity of pixels, intensity * x offset, * y offset, .. etc.
 * bounding box, then pointer to parent spot if this gets merged with
 * another blob (whereapon n will be reset to 0) - n is the number of
 * pixels contributing to the spot.
 *
 * Sized to be 6 words / 24 bytes per spot.
 */

typedef struct spot {
  uint32_t i_sum;
  uint32_t ix_sum;
  uint32_t iy_sum;
  uint16_t x0, x1, y0, y1;
  uint16_t parent, n;
} spot;

// global variables

uint32_t *im = NULL;
uint32_t *m_sat = NULL;
uint32_t *i_sat = NULL;
uint32_t *i2_sat = NULL;

#define MAX_SPOTS 1024

spot *spots = NULL;
uint32_t nspots = 0;

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

  // allocate room for 1024 spots
  spots = (spot *)m_malloc(sizeof(spot) * MAX_SPOTS);
  nspots = 0;

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
  m_free(spots);

  im = m_sat = i_sat = i2_sat = NULL;
  spots = (spot *)NULL;

  return 0;
}

uint32_t signal_filter_reset(void) {
  uint32_t result = nspots - 1;
  nspots = 0;
  return result;
}

int signal_filter_row(uint16_t *io_row) {
  float sigma_b = 6.0f, sigma_s = 3.0f;
  uint32_t knl2 = 2 * knl + 1;

  int nsignal = 0;

  // spot[0] won't be useful later on since setting the index to this
  // will be the same as a non-signal pixel

  spot no_spot;
  no_spot.i_sum = 0;
  no_spot.ix_sum = 0;
  no_spot.iy_sum = 0;
  no_spot.n = 0;
  no_spot.parent = 0;
  no_spot.x0 = 0;
  no_spot.x1 = 0;
  no_spot.y0 = 0;
  no_spot.y1 = 0;

  spots[0] = no_spot;

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
      float bg_rhs = i_sum * sigma_b * (float)sqrtf((float)2 * (m_sum - 1));
      uint16_t background = bg_lhs > bg_rhs;
      float fg_lhs = (float)m_sum * p - (float)i_sum;
      float fg_rhs = sigma_s * (float)sqrtf((float)i_sum * m_sum);
      uint16_t foreground = fg_lhs > fg_rhs;
      signal = background && foreground;
    }
    io_row[j] = signal;
    nsignal += signal;

    // perform connected component analysis - replaces im[i*width + j]
    // pixel value with spot id or 0
    if (signal) {
      uint16_t above = i > 0 ? im[i * width + j - width] : 0;
      uint16_t left = j > 0 ? im[i * width + j - 1] : 0;

      while (spots[above].parent > 0) {
        above = spots[above].parent;
      }
      while (spots[left].parent > 0) {
        left = spots[left].parent;
      }
      if (above == 0 && left == 0) {
        // create a new spot record
        nspots++;
        spots[nspots].i_sum = p;
        spots[nspots].ix_sum = p * j;
        spots[nspots].iy_sum = p * i;
        spots[nspots].n = 1;
        spots[nspots].x0 = j;
        spots[nspots].x1 = j;
        spots[nspots].y0 = i;
        spots[nspots].y1 = i;
        spots[nspots].parent = 0;
        im[i * width + j] = nspots;
      } else if ((above == left) || (above == 0 && left > 0) ||
                 (above > 0 && left == 0)) {
        // merge with correct one
        uint16_t keep = MAX(above, left);
        spots[keep].i_sum += p;
        spots[keep].ix_sum += p * j;
        spots[keep].iy_sum += p * i;
        spots[keep].n++;
        // given this is 4-connected should never reassign x0, y0?
        spots[keep].x0 = MIN(j, spots[keep].x0);
        spots[keep].x1 = MAX(j, spots[keep].x1);
        spots[keep].y0 = MIN(i, spots[keep].y0);
        spots[keep].y1 = MAX(i, spots[keep].y1);
        im[i * width + j] = keep;
      } else if (above != left) {
        // deal with the collision and add this spot while we are here
        uint16_t keep = MIN(above, left);
        uint16_t reject = MAX(above, left);
        spot r = spots[reject];

        spots[keep].i_sum += r.i_sum;
        spots[keep].ix_sum += r.ix_sum;
        spots[keep].iy_sum += r.iy_sum;
        spots[keep].n += r.n;
        spots[keep].x0 = MIN(r.x0, spots[keep].x0);
        spots[keep].x1 = MAX(r.x1, spots[keep].x1);
        spots[keep].y0 = MIN(r.y0, spots[keep].y0);
        spots[keep].y1 = MAX(r.y1, spots[keep].y1);

        spots[reject] = no_spot;
        spots[reject].parent = keep;

        spots[keep].i_sum += p;
        spots[keep].ix_sum += p * j;
        spots[keep].iy_sum += p * i;
        spots[keep].n++;
        spots[keep].x0 = MIN(j, spots[keep].x0);
        spots[keep].x1 = MAX(j, spots[keep].x1);
        spots[keep].y0 = MIN(i, spots[keep].y0);
        spots[keep].y1 = MAX(i, spots[keep].y1);
        im[i * width + j] = keep;
      }

    } else {
      im[i * width + j] = 0;
    }
  }

  row++;

  if (row == height + knl)
    row = 0;

  return nsignal;
}
