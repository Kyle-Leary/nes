#include "util.h"

u16 convertToLittleEndian16(u16 value) {
  u16 result = ((value & 0xFF) << 8) | ((value >> 8) & 0xFF);
  return result;
}

// helper functions
u32 convertToLittleEndian(u32 value) {
  u32 result = ((value & 0xFF) << 24) | ((value & 0xFF00) << 8) |
               ((value >> 8) & 0xFF00) | ((value >> 24) & 0xFF);
  return result;
}

void mapArraySection(u8 *source, u8 *destination, int sourceStart,
                     int destinationStart, int length) {
  int i;
  for (i = 0; i < length; i++) {
    destination[destinationStart + i] = source[sourceStart + i];
  }
}
