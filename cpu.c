#include "cpu.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>

CPUState *make_cpu_state() {
  CPUState *state = (CPUState *)malloc(sizeof(CPUState));

  state->pc = 0; // to be read from the reset vector, when that gets setup.
  state->a = 0;
  state->x = 0;
  state->y = 0;
  state->status = 0b00100000; // flip the unused bit? does it matter?

  state->shutting_down = 0;

  return state;
}

void clean_cpu_state(CPUState *state) {}

EmuState *make_emu_state() {
  EmuState *state = (EmuState *)malloc(sizeof(EmuState));
  state->cpu_state = make_cpu_state();
  state->ram = (u8 *)malloc(0xFFFF);
  state->rom = NULL;
  return state;
}

void clean_emu_state(EmuState *state) { clean_cpu_state(state->cpu_state); }

void debug_print(EmuState *state) {
  if (state == NULL) {
    printf("EmuState is NULL\n");
    return;
  }

  printf("EmuState:\n");

  // Print CPUState info
  printf("  CPUState:\n");
  if (state->cpu_state != NULL) {
    printf("    PC: 0x%04X\n", state->cpu_state->pc);
    printf("    A: 0x%02X\n", state->cpu_state->a);
    printf("    X: 0x%02X\n", state->cpu_state->x);
    printf("    Y: 0x%02X\n", state->cpu_state->y);
    printf("    Status: 0b%08B\n", state->cpu_state->status);
    printf("    Shutting down: %s\n",
           state->cpu_state->shutting_down ? "Yes" : "No");
  } else {
    printf("    CPUState is NULL\n");
  }

  // Print RAM and ROM info
  if (state->ram != NULL) {
    printf("  RAM: %p\n", state->ram); // %p formats a pointer
  } else {
    printf("  RAM is NULL\n");
  }
  if (state->rom != NULL) {
    printf("  ROM: %p\n", state->rom);
  } else {
    printf("  ROM is NULL\n");
  }

  // Print PRG and CHR size info
  printf("  PRG size: %u * 16kb\n", state->prg_size);
  printf("  CHR size: %u * 8kb\n", state->chr_size);
}

/// STATUS HELPERS
// flip the passed statusbit
void toggle_status(CPUState *cs, StatusBit s) {
  cs->status ^= s; // since the enum IS the bit value, this works.
} // XOR is toggling.

void unset_status(CPUState *cs, StatusBit s) {
  cs->status &= ~s;
} // AND with the complement to disable.

void set_status(CPUState *cs, StatusBit s) { cs->status |= s; }

u8 is_status_set(StatusBit s, CPUState *cs) {
  return ((cs->status & s) != 0);
} // the is_set function, but for the status register and its special enum
  // layout.

// 1 if set, 0 if not set.
// is which_bit on byte set?
u8 is_set(u8 which_bit, u8 byte) {
  return ((byte & (1 << which_bit)) != 0);
} // if it's nonzero at all, that's evidence that the bit is set.

// set zero if the target is zero,
// and set negative if bit 7 (leftmost) of target is set.
void neg_and_zero(CPUState *cs, u8 target) {
  if (target == 0) // if the byte is zero...
    set_status(cs, Zero);

  if (is_set(7, target)) // if the leftmost bit is set...
    set_status(cs, Negative);
}

// so it has come to this
#define CS state->cpu_state
#define PC state->cpu_state->pc
#define SP state->cpu_state->csp
#define A state->cpu_state->a
#define X state->cpu_state->x
#define Y state->cpu_state->y
#define STATUS state->cpu_state->status

// helper for defining the function headers.
#define INST(name) void name(EmuState *state, Args *args)

// logic functions.
// CPU instructions.
INST(adc) {}
INST(and) {}
INST(asl) {}
INST(bcc) {}
INST(bcs) {}
INST(beq) {}
INST(bit) {}
INST(bmi) {}
INST(bne) {}
INST(bpl) {}
INST(brk) { state->cpu_state->shutting_down = 1; }
INST(bvc) {}
INST(bvs) {}
INST(clc) {}
INST(cld) {}
INST(cli) {}
INST(clv) {}
INST(cmp) {}
INST(cpx) {}
INST(cpy) {}
INST(dec) {}
INST(dex) {}
INST(dey) {}
INST(eor) {}
INST(inc) {}
INST(inx) {}
INST(iny) {}
INST(jmp) {}
INST(jsr) {}
INST(lda) { A = args->immediate; }
INST(ldx) {}
INST(ldy) {}
INST(lsr) {}
INST(nop) {}
INST(ora) {}
INST(pha) {}
INST(php) {}
INST(pla) {}
INST(plp) {}
INST(rol) {}
INST(ror) {}
INST(rti) {}
INST(rts) {}
INST(sbc) {}
INST(sec) {}
INST(sed) {}
INST(sei) {}
INST(sta) {}
INST(stx) {}
INST(sty) {}
INST(tax) {
  X = A;
  neg_and_zero(CS, X);
} // X = A, A -> X
INST(tay) {
  Y = A;
  neg_and_zero(CS, Y);
} // Y = A, A -> Y, and etc...
INST(tsx) {
  X = STATUS;
  neg_and_zero(CS, X);
}
INST(txa) {
  A = X;
  neg_and_zero(CS, A);
}
INST(txs) {}
INST(tya) {}

// takes in an instruction function and a mode.
// this handles the addressing mode abstractions, giving the function
// the right args and incrementing the pc the right amount to the next
// instruction.
void call(void (*function)(EmuState *, Args *), AddrMode mode, EmuState *es) {
  CPUState *cs = es->cpu_state;
  Args *arg = (Args *)malloc(2);

  // increment to either the first arg byte or the next instruction.
  cs->pc++;

  switch (mode) {
  case None:
    // no arg, do nothing
    break;
  case Immediate:
    arg->immediate = es->ram[cs->pc];
    cs->pc++;
    break;
  default:
    printf("Invalid addressing mode.");
    break;
  }

  function(es, arg);
}

// handlers and etc logic
void handle_instruction(EmuState *state) {
  CPUState *cs = state->cpu_state;
  u8 base_instruction = state->ram[cs->pc];

  if (is_debug)
    printf("Running 0x%02X...\n", base_instruction);

#define C(fn, addrmode) call(fn, addrmode, state)

  // then, specify the full opcode table, including the different addressing
  // modes.
  switch (base_instruction) {
  case 0x00:
    C(brk, None);
    break;
  case 0x01:
    C(ora, IndexedIndirect);
    break;
  case 0x05:
    C(ora, ZP);
    break;
  case 0x06:
    C(asl, ZP);
    break;
  case 0x08:
    C(php, None);
    break;
  case 0x09:
    C(ora, Immediate);
    break;
  case 0x0A:
    C(asl, None);
    break;
  case 0x0D:
    C(ora, Abs);
    break;
  case 0x0E:
    C(asl, Abs);
    break;
  case 0x10:
    C(bpl, Relative);
    break;
  case 0x11:
    C(ora, IndirectIndexed);
    break;
  case 0x15:
    C(ora, ZPX);
    break;
  case 0x16:
    C(asl, ZPX);
    break;
  case 0x18:
    C(clc, None);
    break;
  case 0x19:
    C(ora, AbsY);
    break;
  case 0x1D:
    C(ora, AbsX);
    break;
  case 0x1E:
    C(asl, AbsX);
    break;
  case 0x20:
    C(jsr, Abs);
    break;
  case 0x21:
    C(and, IndexedIndirect);
    break;
  case 0x24:
    C(bit, ZP);
    break;
  case 0x25:
    C(and, ZP);
    break;
  case 0x26:
    C(rol, ZP);
    break;
  case 0x28:
    C(plp, None);
    break;
  case 0x29:
    C(and, Immediate);
    break;
  case 0x2A:
    C(rol, None);
    break;
  case 0x2C:
    C(bit, Abs);
    break;
  case 0x2D:
    C(and, Abs);
    break;
  case 0x2E:
    C(rol, Abs);
    break;
  case 0x30:
    C(bmi, Relative);
    break;
  case 0x31:
    C(and, IndirectIndexed);
    break;
  case 0x35:
    C(and, ZPX);
    break;
  case 0x36:
    C(rol, ZPX);
    break;
  case 0x38:
    C(sec, None);
    break;
  case 0x39:
    C(and, AbsY);
    break;
  case 0x3D:
    C(and, AbsX);
    break;
  case 0x3E:
    C(rol, AbsX);
    break;
  case 0x40:
    C(rti, None);
    break;
  case 0x41:
    C(eor, IndexedIndirect);
    break;
  case 0x45:
    C(eor, ZP);
    break;
  case 0x46:
    C(lsr, ZP);
    break;
  case 0x48:
    C(pha, None);
    break;
  case 0x49:
    C(eor, Immediate);
    break;
  case 0x4A:
    C(lsr, None);
    break;
  case 0x4C:
    C(jmp, Abs);
    break;
  case 0x4D:
    C(eor, Abs);
    break;
  case 0x4E:
    C(lsr, Abs);
    break;
  case 0x50:
    C(bvc, Relative);
    break;
  case 0x51:
    C(eor, IndirectIndexed);
    break;
  case 0x55:
    C(eor, ZPX);
    break;
  case 0x56:
    C(lsr, ZPX);
    break;
  case 0x58:
    C(cli, None);
    break;
  case 0x59:
    C(eor, AbsY);
    break;
  case 0x5D:
    C(eor, AbsX);
    break;
  case 0x5E:
    C(lsr, AbsX);
    break;
  case 0x60:
    C(rts, None);
    break;
  case 0x61:
    C(adc, IndexedIndirect);
    break;
  case 0x65:
    C(adc, ZP);
    break;
  case 0x66:
    C(ror, ZP);
    break;
  case 0x68:
    C(pla, None);
    break;
  case 0x69:
    C(adc, Immediate);
    break;
  case 0x6A:
    C(ror, None);
    break;
  case 0x6C:
    C(jmp, None);
    break;
  case 0x6D:
    C(adc, Abs);
    break;
  case 0x6E:
    C(ror, Abs);
    break;
  case 0x70:
    C(bvs, Relative);
    break;
  case 0x71:
    C(adc, IndirectIndexed);
    break;
  case 0x75:
    C(adc, ZPX);
    break;
  case 0x76:
    C(ror, ZPX);
    break;
  case 0x78:
    C(sei, None);
    break;
  case 0x79:
    C(adc, AbsY);
    break;
  case 0x7D:
    C(adc, AbsX);
    break;
  case 0x7E:
    C(ror, AbsX);
    break;
  case 0x81:
    C(sta, IndexedIndirect);
    break;
  case 0x84:
    C(sty, ZP);
    break;
  case 0x85:
    C(sta, ZP);
    break;
  case 0x86:
    C(stx, ZP);
    break;
  case 0x88:
    C(dey, None);
    break;
  case 0x8A:
    C(txa, None);
    break;
  case 0x8C:
    C(sty, Abs);
    break;
  case 0x8D:
    C(sta, Abs);
    break;
  case 0x8E:
    C(stx, Abs);
    break;
  case 0x90:
    C(bcc, Relative);
    break;
  case 0x91:
    C(sta, IndirectIndexed);
    break;
  case 0x94:
    C(sty, ZPX);
    break;
  case0x95:
    C(sta, ZPX);
    break;
  case 0x96:
    C(stx, ZPY);
    break;
  case 0x98:
    C(tya, None);
    break;
  case 0x99:
    C(sta, AbsY);
    break;
  case 0x9A:
    C(txs, None);
    break;
  case 0x9D:
    C(sta, AbsX);
    break;
  case 0xA0:
    C(ldy, Immediate);
    break;
  case 0xA1:
    C(lda, IndexedIndirect);
    break;
  case 0xA2:
    C(ldx, Immediate);
    break;
  case 0xA4:
    C(ldy, ZP);
    break;
  case 0xA5:
    C(lda, ZP);
    break;
  case 0xA6:
    C(ldx, ZP);
    break;
  case 0xA8:
    C(tay, None);
    break;
  case 0xA9:
    C(lda, Immediate);
    break;
  case 0xAA:
    C(tax, None);
    break;
  case 0xAC:
    C(ldy, Abs);
    break;
  case 0xAD:
    C(lda, Abs);
    break;
  case 0xAE:
    C(ldx, Abs);
    break;
  case 0xB0:
    C(bcs, Relative);
    break;
  case 0xB1:
    C(lda, IndirectIndexed);
    break;
  case 0xB4:
    C(ldy, ZPX);
    break;
  case 0xB5:
    C(lda, ZPX);
    break;
  case 0xB6:
    C(ldx, ZPY);
    break;
  case 0xB8:
    C(clv, None);
    break;
  case 0xB9:
    C(lda, AbsY);
    break;
  case 0xBA:
    C(tsx, None);
    break;
  case 0xBC:
    C(ldy, AbsX);
    break;
  case 0xBD:
    C(lda, AbsX);
    break;
  case 0xBE:
    C(ldx, AbsY);
    break;
  case 0xC0:
    C(cpy, Immediate);
    break;
  case 0xC1:
    C(cmp, IndexedIndirect);
    break;
  case 0xC4:
    C(cpy, ZP);
    break;
  case 0xC5:
    C(cmp, ZP);
    break;
  case 0xC6:
    C(dec, ZP);
    break;
  case 0xC8:
    C(iny, None);
    break;
  case 0xC9:
    C(cmp, Immediate);
    break;
  case 0xCA:
    C(dex, None);
    break;
  case 0xCC:
    C(cpy, Abs);
    break;
  case 0xCD:
    C(cmp, Abs);
    break;
  case 0xCE:
    C(dec, Abs);
    break;
  case 0xD0:
    C(bne, Relative);
    break;
  case 0xD1:
    C(cmp, IndirectIndexed);
    break;
  case 0xD5:
    C(cmp, ZPX);
    break;
  case 0xD6:
    C(dec, ZPX);
    break;
  case 0xD8:
    C(cld, None);
    break;
  case 0xD9:
    C(cmp, AbsY);
    break;
  case 0xDD:
    C(cmp, AbsX);
    break;
  case 0xDE:
    C(dec, AbsX);
    break;
  case 0xE0:
    C(cpx, Immediate);
    break;
  case 0xE1:
    C(sbc, IndexedIndirect);
    break;
  case 0xE4:
    C(cpx, ZP);
    break;
  case 0xE5:
    C(sbc, ZP);
    break;
  case 0xE6:
    C(inc, ZP);
    break;
  case 0xE8:
    C(inx, None);
    break;
  case 0xE9:
    C(sbc, Immediate);
    break;
  case 0xEA:
    C(nop, None);
    break;
  case 0xEC:
    C(cpx, Abs);
    break;
  case 0xED:
    C(sbc, Abs);
    break;
  case 0xEE:
    C(inc, Abs);
    break;
  case 0xF0:
    C(beq, Relative);
    break;
  case 0xF1:
    C(sbc, IndirectIndexed);
    break;
  case 0xF5:
    C(sbc, ZPX);
    break;
  case 0xF6:
    C(inc, ZPX);
    break;
  case 0xF8:
    C(sed, None);
    break;
  case 0xF9:
    C(sbc, AbsY);
    break;
  case 0xFD:
    C(sbc, AbsX);
    break;
  case 0xFE:
    C(inc, AbsX);
    break;
  case 0xFF: // print debug, not real or used opcode.
    C(nop, None);
    if (is_debug)
      debug_print(state);
    break;
  default:
    printf("Invalid opcode detected (%02X).\n", base_instruction);
    break;
  }
}

// NOW, DEFINE THE EXPOSED MODULE FUNCTIONS (and state variables)

EmuState *emu_state = NULL;

// main does the parsing and file handling, we use the FILE* to read
// into the emu->rom field
void cpu_init(FILE *rom_file) {
  // open the global state field, not a local one.
  emu_state = make_emu_state();

  // Find the rom_file size
  fseek(rom_file, 0, SEEK_END);
  long rom_file_size = ftell(rom_file);
  rewind(rom_file);

  // Allocate memory to store the file contents
  emu_state->rom = (u8 *)malloc(rom_file_size);

  // Read the file byte by byte
  int byte;
  long index = 0;
  while ((byte = fgetc(rom_file)) != EOF) {
    emu_state->rom[index++] = (u8)byte;
  }

  // done reading, parse the header.
  // it will read in BE? why do we need to convert it? i'm so confused??
  u32 *first_four_bytes = (u32 *)emu_state->rom;
  if (convertToLittleEndian(*first_four_bytes) != ines_magic) {
    printf("Magic number does not match, exiting...\n");
    printf("Your magic: %08X, ines_magic: %08X.\n", *first_four_bytes,
           ines_magic);
    // TODO: exit here
  }

  emu_state->prg_size = emu_state->rom[4];
  emu_state->chr_size = emu_state->rom[5];

  printf("Parsed header. Found a rom with proper magic, a PRG-ROM size of %d * "
         "16kb, and CHR-ROM of %d * 8kb.\n",
         emu_state->prg_size, emu_state->chr_size);

  // now, map the prg-rom into RAM.
  // assume 16kb for now, map into the 0x8000 region in memory.
  mapArraySection(emu_state->rom, emu_state->ram, 0x0010, 0x8000, 0x4000);

  // 0x8000, little endian is our starting postition.
  // TODO: do we have to read this from the header? what's up with the reset
  // vecs?
  emu_state->ram[0xFFFC] = 0x00;
  emu_state->ram[0xFFFD] = 0x80;
  // TODO: handle the irq vector.

  // TODO: read from the ram.
  emu_state->cpu_state->pc = 0x8000;
}

void cpu_update(u8 *is_running) {
  // lol
  if (emu_state->cpu_state->pc == 0xFFFF) {
    emu_state->cpu_state->pc = 0x8000;
  }
  handle_instruction(emu_state);

  printf("cpu: %d", !emu_state->cpu_state->shutting_down);
  *is_running = !emu_state->cpu_state->shutting_down;
}

void cpu_clean() { clean_emu_state(emu_state); }
