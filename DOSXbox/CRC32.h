#pragma once

#include "External.h"

class CRC32 {
  public:
    static uint32_t Calculate(const uint8_t* buffer, uint32_t size);
  private:
    static void Calculate(const uint8_t* buffer, uint32_t size, uint32_t& crc);
};
