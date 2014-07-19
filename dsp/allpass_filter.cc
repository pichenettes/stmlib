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
// Chain of all-pass filter.

#include "stmlib/dsp/allpass_filter.h"

#include <algorithm>

namespace stmlib {

using namespace std;

void AllPassFilter::Init(float* buffer, int32_t size) {
  buffer_ = buffer;
  ptr_ = 0;
  size_ = size;
  fill(&buffer_[0], &buffer_[size_], 0.0f);
}
   
void AllPassFilter::Process(float* input_output, size_t size, size_t stride) {
  while (size--) {
    float input = *input_output;
    float read = buffer_[ptr_];
    float write = input + read * -gain_;
    float output = gain_ * write + read;
    *input_output = output;
    buffer_[ptr_] = write;
    ++ptr_;
    if (ptr_ >= size_) {
      ptr_ = 0;
    }
    input_output += stride;
  }
}

}  // namespace stmlib
