// Copyright 2012 Olivier Gillet.
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

#ifndef STMLIB_STMLIB_H_
#define STMLIB_STMLIB_H_

#include <inttypes.h>
#include <stddef.h>

#ifndef NULL
#define NULL 0
#endif

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

template<bool b>
inline void StaticAssertImplementation() {
	char static_assert_size_mismatch[b] = { 0 };
}
 
#define STATIC_ASSERT(expression) StaticAssertImplementation<(expression)>()

typedef union {
  uint16_t value;
  uint8_t bytes[2];
} Word;

typedef union {
  uint32_t value;
  uint16_t words[2];
  uint8_t bytes[4];
} LongWord;

struct uint24_t {
  uint16_t integral;
  uint8_t fractional;
};

struct uint24c_t {
  uint8_t carry;
  uint16_t integral;
  uint8_t fractional;
};

template<uint32_t a, uint32_t b, uint32_t c, uint32_t d>
struct FourCC {
  static const uint32_t value = (((((d << 8) | c) << 8) | b) << 8) | a;
};

namespace stmlib {

template<uint8_t size>
struct DataTypeForSize {
  typedef uint16_t Type;
};

template<> struct DataTypeForSize<1> { typedef uint8_t Type; };
template<> struct DataTypeForSize<2> { typedef uint8_t Type; };
template<> struct DataTypeForSize<3> { typedef uint8_t Type; };
template<> struct DataTypeForSize<4> { typedef uint8_t Type; };
template<> struct DataTypeForSize<5> { typedef uint8_t Type; };
template<> struct DataTypeForSize<6> { typedef uint8_t Type; };
template<> struct DataTypeForSize<7> { typedef uint8_t Type; };
template<> struct DataTypeForSize<8> { typedef uint8_t Type; };

enum DataOrder {
  MSB_FIRST = 0,
  LSB_FIRST = 1
};

enum DigitalValue {
  LOW = 0,
  HIGH = 1
};

// Some classes (SPI, shift register) have a notion of communication session -
// Begin is called, several R/W are done, and then End is called to pull high
// a chip select or latch line. This template ensures that any path leaving a
// block of code will release the resource.
template<typename T>
class scoped_resource {
 public:
  scoped_resource() {
    T::Begin();
  }
  
  ~scoped_resource() {
    T::End();
  }
};

}  // namespace stmlib

#endif   // STMLIB_STMLIB_H_
