// Copyright 2014 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// Conversion from semitones to frequency ratio.

#ifndef STMLIB_DSP_UNITS_H_
#define STMLIB_DSP_UNITS_H_

#include "stmlib/stmlib.h"
#include "stmlib/dsp/dsp.h"

namespace stmlib {

extern const float lut_pitch_ratio_high[257];
extern const float lut_pitch_ratio_low[257];

inline float SemitonesToRatio(float semitones) {
  float pitch = semitones + 128.0f;
  MAKE_INTEGRAL_FRACTIONAL(pitch)

  return lut_pitch_ratio_high[pitch_integral] * \
      lut_pitch_ratio_low[static_cast<int32_t>(pitch_fractional * 256.0f)];
}

inline void TimbreControl(
    float amount,
    float min_frequency,
    float* lp_f, float* hp_f) {
  float f = amount - 0.5f;
  f = f * f * 4.0f;
  f = amount < 0.5f ? 1.0f - f : f;
  float f_cut = min_frequency * SemitonesToRatio(112.0f * f);
  if (f_cut >= 0.49f) {
    f_cut = 0.49f;
  }
  if (amount < 0.5f) {
    *lp_f = f_cut;
    *hp_f = min_frequency;
  } else {
    *lp_f = 0.49f;
    *hp_f = f_cut;
  }
}

}  // namespace stmlib

#endif  // STMLIB_DSP_UNITS_H_
