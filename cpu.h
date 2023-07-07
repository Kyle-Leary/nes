#pragma once

#include "defines.h"
#include <stdio.h>

#define ines_magic 0x4E45531A

typedef union { // args can either be some u16 entity,
  // or a u8 entity. pass this to get around the weirdness.
  u8 immediate;
  u16 address;
} Args;

// enums and defines
typedef enum AddrMode { // the addressing mode for each instruction.
  None,                 // AKA implicit addressing.
  Immediate,
  Abs,
  AbsX,
  AbsY,
  ZP,
  ZPX,
  ZPY,
  Relative,
  IndexedIndirect,
  IndirectIndexed,
} AddrMode;

typedef enum StatusBit {
  Carry = (1 << 0),
  Zero = (1 << 1),
  Interrupt = (1 << 2),
  Decimal = (1 << 3),
  Break = (1 << 4),
  Unused = (1 << 5), // "unused, often set to 1."
  Overflow = (1 << 6),
  Negative =
      (1 << 7), // (1 << 7) == 0b1000 0000, it's just flipping the nth bit.
} StatusBit;

typedef struct CPUState {
  u16 pc;
  u8 sp; // is the sp 8 bit?

  u8 a;
  u8 x;
  u8 y;
  u8 status;

  u8 shutting_down; // for BRK.
} CPUState;

// our overall stateful object for the emulator core.
// cleaning this should clean EVERYTHING else.
typedef struct EmuState {
  CPUState *cpu_state;

  u8 *ram;
  u8 *rom;

  u8 prg_size; // both straight from the header.
  u8 chr_size;
} EmuState;

// static module instances.
// we'll hide the constructor, and only expose the instance itself
// and the structure to the rest of the program.
extern EmuState *emu_state;

void cpu_init(FILE *rom_file);
void cpu_update(u8 *is_running);
void cpu_clean();
