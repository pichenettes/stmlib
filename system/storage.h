// Copyright 2012 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef STMLIB_SYSTEM_STORAGE_H_
#define STMLIB_SYSTEM_STORAGE_H_

#include <stm32f10x_conf.h>
#include "stmlib/stmlib.h"

#if defined (STM32F10X_LD) || defined (STM32F10X_MD)
  #define PAGE_SIZE  (uint16_t)0x400  /* Page size = 1KByte */
#elif defined (STM32F10X_HD) || defined (STM32F10X_CL)
  #define PAGE_SIZE  (uint16_t)0x800  /* Page size = 2KByte */
#endif

namespace stmlib {

template<uint32_t last_address = 0x8020000>
class Storage {
 public:
  enum {
    FLASH_STORAGE_BASE = last_address - PAGE_SIZE
  };
  
  template<typename T>
  static void Save(const T& data) {
    FLASH_Unlock();
    FLASH_ErasePage(FLASH_STORAGE_BASE);
    
    // Write argument.
    const uint32_t* words = (const uint32_t*)(&data);
    size_t size = sizeof(T);
    uint32_t address = FLASH_STORAGE_BASE;
    while (size >= 4) {
      FLASH_ProgramWord(address, *words++);
      address += 4;
      size -= 4;
    }
    // If the size is not a multiple of 4, write tail.
    if (size) {
      FLASH_ProgramWord(address, *(const uint16_t*)(words));
    }
    
    // Write checksum.
    uint16_t checksum = Checksum((const void*)(&data), sizeof(T));
    FLASH_ProgramWord(FLASH_STORAGE_BASE + sizeof(T), checksum);
  };

  template<typename T>
  static bool Load(T* data) {
    memcpy(data, (void*)(FLASH_STORAGE_BASE), sizeof(T));
    uint16_t checksum = (*(uint16_t*)(FLASH_STORAGE_BASE + sizeof(T)));
    return checksum == Checksum(data, sizeof(T));
  };
  
 private:
  static uint16_t Checksum(const void* data, uint16_t size) {
    uint16_t s = 0;
    const uint8_t* d = static_cast<const uint8_t*>(data);
    while (size--) {
      s += *d++;
    }
    return s;
  }
};

};  // namespace edges

#endif  // STMLIB_SYSTEM_STORAGE_H_
