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
// Zero-delay-feedback filters (one pole and SVF).
// Naive SVF.

#ifndef STMLIB_DSP_FILTER_H_
#define STMLIB_DSP_FILTER_H_

#include "stmlib/stmlib.h"

#include <cmath>

namespace stmlib {

enum FilterMode {
  FILTER_MODE_LOW_PASS,
  FILTER_MODE_BAND_PASS,
  FILTER_MODE_BAND_PASS_NORMALIZED,
  FILTER_MODE_HIGH_PASS
};

enum FrequencyApproximation {
  FREQUENCY_EXACT,
  FREQUENCY_ACCURATE,
  FREQUENCY_FAST,
  FREQUENCY_DIRTY
};

#define M_PI_F float(M_PI)
#define M_PI_POW_2 M_PI * M_PI
#define M_PI_POW_3 M_PI_POW_2 * M_PI
#define M_PI_POW_5 M_PI_POW_3 * M_PI_POW_2
#define M_PI_POW_7 M_PI_POW_5 * M_PI_POW_2
#define M_PI_POW_9 M_PI_POW_7 * M_PI_POW_2
#define M_PI_POW_11 M_PI_POW_9 * M_PI_POW_2

class OnePole {
 public:
  OnePole() { }
  ~OnePole() { }
  
  void Init() {
    set_f<FREQUENCY_DIRTY>(0.01f);
    Reset();
  }
  
  void Reset() {
    state_ = 0.0f;
  }
  
  template<FrequencyApproximation approximation>
  static inline float tan(float f) {
    if (approximation == FREQUENCY_EXACT) {
      // Clip coefficient to about 100.
      f = f < 0.497f ? f : 0.497f;
      return tanf(M_PI * f);
    } else if (approximation == FREQUENCY_DIRTY) {
      // Optimized for frequencies below 8kHz.
      const float a = 3.736e-01 * M_PI_POW_3;
      return f * (M_PI_F + a * f * f);
    } else if (approximation == FREQUENCY_FAST) {
      // The usual tangent approximation uses 3.1755e-01 and 2.033e-01, but
      // the coefficients used here are optimized to minimize error for the
      // 16Hz to 16kHz range, with a sample rate of 48kHz.
      const float a = 3.260e-01 * M_PI_POW_3;
      const float b = 1.823e-01 * M_PI_POW_5;
      float f2 = f * f;
      return f * (M_PI_F + f2 * (a + b * f2));
    } else if (approximation == FREQUENCY_ACCURATE) {
      // These coefficients don't need to be tweaked for the audio range.
      const float a = 3.333314036e-01 * M_PI_POW_3;
      const float b = 1.333923995e-01 * M_PI_POW_5;
      const float c = 5.33740603e-02 * M_PI_POW_7;
      const float d = 2.900525e-03 * M_PI_POW_9;
      const float e = 9.5168091e-03 * M_PI_POW_11;
      float f2 = f * f;
      return f * (M_PI_F + f2 * (a + f2 * (b + f2 * (c + f2 * (d + f2 * e)))));
    }
  }
  
  // Set frequency and resonance from true units. Various approximations
  // are available to avoid the cost of tanf.
  template<FrequencyApproximation approximation>
  inline void set_f(float f) {
    g_ = tan<approximation>(f);
    gi_ = 1.0f / (1.0f + g_);
  }
  
  template<FilterMode mode>
  inline float Process(float in) {
    float lp;
    lp = (g_ * in + state_) * gi_;
    state_ = g_ * (in - lp) + lp;

    if (mode == FILTER_MODE_LOW_PASS) {
      return lp;
    } else if (mode == FILTER_MODE_HIGH_PASS) {
      return in - lp;
    } else {
      return 0.0f;
    }
  }
  
 private:
  float g_;
  float gi_;
  float state_;
  
  DISALLOW_COPY_AND_ASSIGN(OnePole);
};



class Svf {
 public:
  Svf() { }
  ~Svf() { }
  
  void Init() {
    set_f_q<FREQUENCY_DIRTY>(0.01f, 100.0f);
    Reset();
  }
  
  void Reset() {
    state_1_ = state_2_ = 0.0f;
  }
  
  // Copy settings from another filter.
  inline void set(const Svf& f) {
    g_ = f.g();
    r_ = f.r();
    h_ = f.h();
  }

  // Set all parameters from LUT.
  inline void set_g_r_h(float g, float r, float h) {
    g_ = g;
    r_ = r;
    h_ = h;
  }
  
  // Set frequency and resonance coefficients from LUT, adjust remaining
  // parameter.
  inline void set_g_r(float g, float r) {
    g_ = g;
    r_ = r;
    h_ = 1.0f / (1.0f + r_ * g_ + g_ * g_);
  }

  // Set frequency from LUT, resonance in true units, adjust the rest.
  inline void set_g_q(float g, float resonance) {
    g_ = g;
    r_ = 1.0f / resonance;
    h_ = 1.0f / (1.0f + r_ * g_ + g_ * g_);
  }

  // Set frequency and resonance from true units. Various approximations
  // are available to avoid the cost of tanf.
  template<FrequencyApproximation approximation>
  inline void set_f_q(float f, float resonance) {
    g_ = OnePole::tan<approximation>(f);
    r_ = 1.0f / resonance;
    h_ = 1.0f / (1.0f + r_ * g_ + g_ * g_);
  }
  
  template<FilterMode mode>
  inline float Process(float in) {
    float hp, bp, lp;
    hp = (in - r_ * state_1_ - g_ * state_1_ - state_2_) * h_;
    bp = g_ * hp + state_1_;
    state_1_ = g_ * hp + bp;
    lp = g_ * bp + state_2_;
    state_2_ = g_ * bp + lp;
    
    if (mode == FILTER_MODE_LOW_PASS) {
      return lp;
    } else if (mode == FILTER_MODE_BAND_PASS) {
      return bp;
    } else if (mode == FILTER_MODE_BAND_PASS_NORMALIZED) {
      return bp * r_;
    } else if (mode == FILTER_MODE_HIGH_PASS) {
      return hp;
    }
  }
  
  template<FilterMode mode>
  inline void Process(const float* in, float* out, size_t size) {
    float hp, bp, lp;
    float state_1 = state_1_;
    float state_2 = state_2_;
    
    while (size--) {
      hp = (*in - r_ * state_1 - g_ * state_1 - state_2) * h_;
      bp = g_ * hp + state_1;
      state_1 = g_ * hp + bp;
      lp = g_ * bp + state_2;
      state_2 = g_ * bp + lp;
    
      float value;
      if (mode == FILTER_MODE_LOW_PASS) {
        value = lp;
      } else if (mode == FILTER_MODE_BAND_PASS) {
        value = bp;
      } else if (mode == FILTER_MODE_BAND_PASS_NORMALIZED) {
        value = bp * r_;
      } else if (mode == FILTER_MODE_HIGH_PASS) {
        value = hp;
      }
      
      *out = value;
      ++out;
      ++in;
    }
    state_1_ = state_1;
    state_2_ = state_2;
  }
  
  inline void ProcessMultimode(
      const float* in,
      float* out,
      size_t size,
      float mode) {
    float hp, bp, lp;
    float state_1 = state_1_;
    float state_2 = state_2_;
    
    mode *= mode;
    
    float hp_gain = mode < 0.5f ? mode * 2.0f : 2.0f - mode * 2.0f;
    float lp_gain = mode < 0.5f ? 1.0f - mode * 2.0f : 0.0f;
    float bp_gain = mode < 0.5f ? 0.0f : mode * 2.0f - 1.0f;
    
    while (size--) {
      hp = (*in - r_ * state_1 - g_ * state_1 - state_2) * h_;
      bp = g_ * hp + state_1;
      state_1 = g_ * hp + bp;
      lp = g_ * bp + state_2;
      state_2 = g_ * bp + lp;
      *out = hp_gain * hp + bp_gain * bp + lp_gain * lp;
      ++in;
      ++out;
    }
    state_1_ = state_1;
    state_2_ = state_2;
  }
  
  template<FilterMode mode>
  inline void Process(
      const float* in, float* out_1, float* out_2, size_t size,
      float gain_1, float gain_2) {
    float hp, bp, lp;
    float state_1 = state_1_;
    float state_2 = state_2_;
    
    while (size--) {
      hp = (*in - r_ * state_1 - g_ * state_1 - state_2) * h_;
      bp = g_ * hp + state_1;
      state_1 = g_ * hp + bp;
      lp = g_ * bp + state_2;
      state_2 = g_ * bp + lp;
    
      float value;
      if (mode == FILTER_MODE_LOW_PASS) {
        value = lp;
      } else if (mode == FILTER_MODE_BAND_PASS) {
        value = bp;
      } else if (mode == FILTER_MODE_BAND_PASS_NORMALIZED) {
        value = bp * r_;
      } else if (mode == FILTER_MODE_HIGH_PASS) {
        value = hp;
      }
      
      *out_1 += value * gain_1;
      *out_2 += value * gain_2;
      ++out_1;
      ++out_2;
      ++in;
    }
    state_1_ = state_1;
    state_2_ = state_2;
  }
  
  inline float g() const { return g_; }
  inline float r() const { return r_; }
  inline float h() const { return h_; }
  
 private:
  float g_;
  float r_;
  float h_;

  float state_1_;
  float state_2_;
  
  DISALLOW_COPY_AND_ASSIGN(Svf);
};



class NaiveSvf {
 public:
  NaiveSvf() { }
  ~NaiveSvf() { }
  
  void Init() {
    set_f_q<FREQUENCY_DIRTY>(0.01f, 100.0f);
    Reset();
  }
  
  void Reset() {
    lp_ = bp_ = 0.0f;
  }
  
  // Set frequency and resonance from true units. Various approximations
  // are available to avoid the cost of sinf.
  template<FrequencyApproximation approximation>
  inline void set_f_q(float f, float resonance) {
    f = f < 0.497f ? f : 0.497f;
    if (approximation == FREQUENCY_EXACT) {
      f_ = 2.0f * sinf(M_PI_F * f);
    } else {
      f_ = 2.0f * M_PI_F * f;
    }
    damp_ = 1.0f / resonance;
  }
  
  template<FilterMode mode>
  inline float Process(float in) {
    float hp, notch, bp_normalized;
    bp_normalized = bp_ * damp_;
    notch = in - bp_normalized;
    lp_ += f_ * bp_;
    hp = notch - lp_;
    bp_ += f_ * hp;
    
    if (mode == FILTER_MODE_LOW_PASS) {
      return lp_;
    } else if (mode == FILTER_MODE_BAND_PASS) {
      return bp_;
    } else if (mode == FILTER_MODE_BAND_PASS_NORMALIZED) {
      return bp_normalized;
    } else if (mode == FILTER_MODE_HIGH_PASS) {
      return hp;
    }
  }

  template<FilterMode mode>
  inline void Process(const float* in, float* out, size_t size, size_t decimate) {
    float hp, notch, bp_normalized;
    float lp = lp_;
    float bp = bp_;
    size_t n = decimate - 1;
    while (size--) {
      bp_normalized = bp * damp_;
      notch = *in++ - bp_normalized;
      lp += f_ * bp;
      hp = notch - lp;
      bp += f_ * hp;
      
      ++n;
      if (n == decimate) {
        if (mode == FILTER_MODE_LOW_PASS) {
          *out++ = lp;
        } else if (mode == FILTER_MODE_BAND_PASS) {
          *out++ = bp;
        } else if (mode == FILTER_MODE_BAND_PASS_NORMALIZED) {
          *out++ = bp_normalized;
        } else if (mode == FILTER_MODE_HIGH_PASS) {
          *out++ = hp;
        }
        n = 0;
      }
    }
    lp_ = lp;
    bp_ = bp;
  }
  
 private:
  float f_;
  float damp_;
  float lp_;
  float bp_;
  
  DISALLOW_COPY_AND_ASSIGN(NaiveSvf);
};


}  // namespace stmlib

#endif  // STMLIB_DSP_FILTER_H_
