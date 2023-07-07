#include "defines.h"

u16 convertToLittleEndian16(u16 value);
u32 convertToLittleEndian(u32 value);
void mapArraySection(u8 *source, u8 *destination, int sourceStart,
                     int destinationStart, int length);
