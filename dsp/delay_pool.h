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
// Group of delay lines sharing the same storage area;

#ifndef STMLIB_DSP_DELAY_POOL_H_
#define STMLIB_DSP_DELAY_POOL_H_

#include "stmlib/stmlib.h"

#include <algorithm>

namespace stmlib {

template<typename T, size_t size, size_t num_delays>
class DelayPool {
 public:
  DelayPool() { }
  ~DelayPool() { }
  
  void Init(const size_t* partition) {
    std::fill(&line_[0], &line_[size], T(0));
    start_[0] = 0;
    for (size_t i = 1; i < num_delays; ++i) {
      start_[i] = start_[i - 1] + partition[i - 1];
    }
    write_ptr_ = 0;
  }
 
  inline void NextSample() {
    --write_ptr_;
    if (write_ptr_ < 0) {
      write_ptr_ += size;
    }
  }
  
  inline void Write(int32_t index, const T& sample) {
    line_[(write_ptr_ + start_[index]) % size] = sample;
  }
  
  inline const T& Read(size_t index, size_t delay) const {
    return line_[(write_ptr_ + start_[index] + delay) % size];
  }
  
 private:
  int32_t write_ptr_;
  int32_t start_[num_delays];
  T line_[size];
  
  DISALLOW_COPY_AND_ASSIGN(DelayPool);
};

}  // namespace stmlib

#endif  // STMLIB_DSP_DELAY_POOL_H_
