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
// All-pass filters.

#ifndef STMLIB_DSP_ALL_PASS_H_
#define STMLIB_DSP_ALL_PASS_H_

#include "stmlib/stmlib.h"

namespace stmlib {

class AllPassFilter {
 public:
  AllPassFilter() { }
  ~AllPassFilter() { }

  void Init(float* buffer, int32_t size);
  void Process(float* input_output, size_t size, size_t stride);
    
  inline void set_gain(float gain) {
    gain_ = gain;
  }
  
  
 private:
  float* buffer_;
  float gain_;
  int32_t ptr_;
  int32_t size_;
  
  DISALLOW_COPY_AND_ASSIGN(AllPassFilter);
};

}  // namespace stmlib

#endif  // STMLIB_DSP_ALL_PASS_H_
