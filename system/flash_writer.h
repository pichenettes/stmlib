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
//
// -----------------------------------------------------------------------------
//
// Helper functions for using the last page of flash for non-volatile storage.

#ifndef STMLIB_SYSTEM_FLASH_WRITER_H_
#define STMLIB_SYSTEM_FLASH_WRITER_H_

#include <stm32f10x_conf.h>

#include "stmlib/stmlib.h"
#include "stmlib/utils/ring_buffer.h"

#define CR_STRT_Set              ((uint32_t)0x00000040)
#define CR_PG_Set                ((uint32_t)0x00000001)
#define CR_PG_Reset              ((uint32_t)0x00001FFE) 
#define CR_PER_Set               ((uint32_t)0x00000002)
#define CR_PER_Reset             ((uint32_t)0x00001FFD)

namespace stmlib {

enum FlashWriterState {
  FLASH_WRITER_STATE_READY,
  FLASH_WRITER_STATE_ERASING_PAGE,
  FLASH_WRITER_STATE_WAITING_FOR_PAGE_ERASE,
  FLASH_WRITER_STATE_WRITING,
  FLASH_WRITER_STATE_WAITING_FOR_WRITE
};

struct FlashWriterJob {
  uint32_t destination_address;
  uint32_t source_address;
  uint32_t size;
};

enum IMMEDIATE_VALUE {
  IMMEDIATE_VALUE_HALF_WORD = 0xffffffff,
  IMMEDIATE_VALUE_WORD = IMMEDIATE_VALUE_HALF_WORD - 2
};

class FlashWriter {
 public:
  FlashWriter() { }
  ~FlashWriter() { }
  
  void Init() {
    state_ = FLASH_WRITER_STATE_READY;
    jobs_.Init();
  }
  
  void Flush() {
    while (jobs_.readable()) Process();
    while (state_ != FLASH_WRITER_STATE_READY) Process();
  }
  
  void Process() {
    switch (state_) {
      case FLASH_WRITER_STATE_READY:
        if (jobs_.readable()) {
          job_ = jobs_.ImmediateRead();
          if (job_.source_address == 0 && job_.size == 0) {
            state_ = FLASH_WRITER_STATE_ERASING_PAGE;
          } else {
            state_ = FLASH_WRITER_STATE_WRITING;
          }
        }
        break;
        
      case FLASH_WRITER_STATE_ERASING_PAGE:
        {
          FLASH_Status status;
          status = FLASH_GetStatus();
          if (status == FLASH_BUSY) {
            // Handle timeout.
          } else if (status == FLASH_COMPLETE) {
            FLASH->CR|= CR_PER_Set;
            FLASH->AR = job_.destination_address; 
            FLASH->CR|= CR_STRT_Set;
            state_ = FLASH_WRITER_STATE_WAITING_FOR_PAGE_ERASE;
          } else {
            state_ = FLASH_WRITER_STATE_READY;
          }
        }
        break;
        
      case FLASH_WRITER_STATE_WAITING_FOR_PAGE_ERASE:
        {
          FLASH_Status status;
          status = FLASH_GetStatus();
          if (status == FLASH_BUSY) {
            // Handle timeout
          } else {
            FLASH->CR &= CR_PER_Reset;
            state_ = FLASH_WRITER_STATE_READY;
          }
        }
        break;
        
      case FLASH_WRITER_STATE_WRITING:
        {
          FLASH_Status status;
          status = FLASH_GetStatus();
          if (status == FLASH_BUSY) {
            // Handle timeout
          } else if (status == FLASH_COMPLETE) {
            FLASH->CR |= CR_PG_Set;
            uint16_t data = 0;
            if (job_.source_address >= IMMEDIATE_VALUE_HALF_WORD) {
              data = job_.size;
            } else {
              uint16_t* address = (uint16_t*)(job_.source_address);
              data = *address;
            }
            *(__IO uint16_t*)(job_.destination_address) = data;
            state_ = FLASH_WRITER_STATE_WAITING_FOR_WRITE;
          } else {
            state_ = FLASH_WRITER_STATE_READY;
          }
          break;
        }
        break;
        
      case FLASH_WRITER_STATE_WAITING_FOR_WRITE:
        {
          FLASH_Status status;
          status = FLASH_GetStatus();
          if (status == FLASH_BUSY) {
            // Handle timeout
          } else {
            FLASH->CR &= CR_PG_Reset;
            if (status == FLASH_COMPLETE) {
              if (job_.source_address == IMMEDIATE_VALUE_WORD) {
                // Write next half-word
                job_.size >>= 16;
                job_.destination_address += 2;
                job_.source_address += 2;
                state_ = FLASH_WRITER_STATE_WRITING;
              } else if (job_.source_address == IMMEDIATE_VALUE_HALF_WORD) {
                // Done
                state_ = FLASH_WRITER_STATE_READY;
              } else if (job_.size > 2) {
                // Write next half-word.
                job_.size -= 2;
                job_.destination_address += 2;
                job_.source_address += 2;
                state_ = FLASH_WRITER_STATE_WRITING;
              } else {
                // Done.
                state_ = FLASH_WRITER_STATE_READY;
              }
            }
          }
        }
        break;
    }
  }
  
  void ErasePage(uint32_t address) {
    FlashWriterJob j;
    j.destination_address = address;
    j.source_address = 0;
    j.size = 0;
    jobs_.Overwrite(j);
  }
  
  void Write(uint32_t address, uint32_t word) {
    FlashWriterJob j;
    j.destination_address = address;
    j.source_address = IMMEDIATE_VALUE_WORD;
    j.size = word;
    jobs_.Overwrite(j);
  }

  void WriteHalfWord(uint32_t address, uint16_t half_word) {
    FlashWriterJob j;
    j.destination_address = address;
    j.source_address = IMMEDIATE_VALUE_HALF_WORD;
    j.size = half_word;
    jobs_.Overwrite(j);
  }
  
  void Write(uint32_t address, uint32_t source, uint32_t size) {
    FlashWriterJob j;
    j.destination_address = address;
    j.source_address = source;
    j.size = size;
    jobs_.Overwrite(j);
  }
  
 private:
  FlashWriterState state_;
   
  RingBuffer<FlashWriterJob, 16> jobs_;
  FlashWriterJob job_;
  
  DISALLOW_COPY_AND_ASSIGN(FlashWriter);
};

};  // namespace stmlib

#endif  // STMLIB_SYSTEM_FLASH_WRITER_H_
