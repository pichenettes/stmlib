// Copyright 2021 Emilie Gillet.
//
// Author: Emilie Gillet (emilie.o.gillet@gmail.com)
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
// A filter that adjusts its behavior depending on the frequency of big jumps
// observed in the input signal:
// - if those jumps are rare, it is assumed that the input signal is steppy
//   (for example, a note sequence). In this case, the filter tracks the median
//   of the input signal over a short amount of time, and then holds this value.
// - if not, defaults to a regular one-pole filter.

#ifndef STMLIB_DSP_ADAPTIVE_FILTER_H_
#define STMLIB_DSP_ADAPTIVE_FILTER_H_

#include "stmlib/stmlib.h"

#include <algorithm>
#include <cmath>

namespace stmlib {

template<int max_observation_period=20>
class AdaptiveFilter {
 public:
  AdaptiveFilter() { }
  ~AdaptiveFilter() { }
  
  enum Mode {
    MODE_ONE_POLE,
    MODE_MEDIAN,
    MODE_HOLD
  };

  void Init(
      float threshold,
      int stable_segment_duration,
      int median_order,
      int observation_period,
      float lp_coefficient) {
    stable_segment_duration_ = stable_segment_duration;
    lp_coefficient_ = lp_coefficient;
    threshold_ = threshold;

    mode_ = MODE_TRACK;
    value_ = 0.0f;
    history_[0] = 0.0f;

    i_ = 0;
    n_ = observation_period;
    k_ = median_order;
    
    stable_segment_counter_ = 0;
  }
  
  inline float Process(float value) {
    if (fabsf(value - value_) > threshold_) {
      bool was_stable = stable_segment_counter_ > stable_segment_duration_;
      mode_ = was_stable ? MODE_MEDIAN : MODE_ONE_POLE;
      i_ = 0;
      stable_segment_counter_ = 0;
    }
    
    if (mode_ == MODE_MEDIAN) {
      history_[i_] = value;
      ++i_;
      
      const int k = std::min(k_, i_);
      float sorted[k];
      std::copy(&history_[i_ - k], &history_[i_], &sorted[0]);
      std::sort(&sorted[0], &sorted[k]);

      value_ = k & 1
          ? sorted[(k - 1) / 2]
          : 0.5f * (sorted[(k - 1) / 2] + sorted[k / 2]);

      if (i_ == n_) {
        mode_ = MODE_HOLD;
      }
    } else if (mode_ == MODE_ONE_POLE) {
      value_ += (value - value_) * lp_coefficient_;
    }
    
    ++stable_segment_counter_;
    
    return value_;
  }

 private:
  int stable_segment_duration_;
  float lp_coefficient_;
  float threshold_;
  
  Mode mode_;
  float value_;
  float history_[max_observation_period];
  
  int i_;  // number of observed samples
  int n_;  // maximum number of samples we can observe
  int k_;  // order of the median filter
  
  int stable_segment_counter_;
  
  DISALLOW_COPY_AND_ASSIGN(AdaptiveFilter);
};


}  // namespace stmlib

#endif  // STMLIB_DSP_ADAPTIVE_FILTER_H_

