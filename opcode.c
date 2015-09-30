#include "consts.h"
#include "helper.h"
#include <stdlib.h>
#include <stdio.h>

extern union reg_accum d;
extern unsigned short pc, x, y, sp, cc;

#define INDEXED(a) if (idx & FIFTH_BIT) {\
    five_bits = (~five_bits) + 1;\
  }\
  return a + five_bits\

#define CREMENT(a) five_bits++;\
  if (five_bits > 8) {\
    five_bits -= 8;\
    five_bits = (~five_bits) + 1;\
  }\
  if (!(idx & FIFTH_BIT)) {\
    a += five_bits;\
    return a;\
  } else {\
    addr = a;\
    a += five_bits;\
    return addr;\
  }\
  break

unsigned short getop_addr(unsigned char opcode) {
  unsigned short *reg, indir_addr, addr;
  unsigned char c, idx;
  char five_bits;
  switch (opcode & LAST_QUARTER) {
  case 1:
    addr = 0;
    addr += readbyte(pc++);
    return addr;
  case 2:
    idx = readbyte(pc++);
    five_bits = idx & SECOND_HALF;
    switch (idx >> 5) {
    case 0:
      INDEXED(x);
    case 1:
      CREMENT(x);
    case 2:
      INDEXED(y);
    case 3:
      CREMENT(y);
    case 4:
      INDEXED(sp);
    case 5:
      CREMENT(sp);
    case 6:
      INDEXED(pc);
    case 7:
      reg = 0;
      switch ((idx >> 3) & LAST_QUARTER) {
      case 0:
	reg = &x;
	break;
      case 1:
	reg = &y;
	break;
      case 2:
	reg = &sp;
	break;
      case 3:
	reg = &pc;
	break;
      default:
	break;
      }
      if (idx & THIRD_BIT) {
	switch (idx & LAST_QUARTER) {
	case 0:
	  return *reg + ACCUM_A;
	  break;
	case 1:
	  return *reg + ACCUM_B;
	  break;
	case 2:
	  return *reg + d.reg;
	  break;
	case 3:
	  addr = readbyte(*reg + d.reg);
	  addr = addr << 8;
	  addr += readbyte(*reg + d.reg + 1);
	  return addr;
	  break;
	default:
	  break;
	}
      } else {
	switch (idx & LAST_QUARTER) {
	case 0:
	  c = readbyte(pc++);
	  return *reg + c;
	  break;
	case 1:
	  c = readbyte(pc++);
	  return *reg - c;
	  break;
	case 2:
	  addr = readbyte(pc++);
	  addr = addr << 8;
	  addr += readbyte(pc++);
	  return *reg + addr;
	  break;
	case 3:
	  addr = readbyte(pc++);
	  addr = addr << 8;
	  addr += readbyte(pc++);
	  indir_addr = readbyte(*reg + addr);
	  indir_addr = indir_addr << 8;
	  indir_addr += readbyte(*reg + addr + 1);
	  return indir_addr;
	  break;
	default:
	  break;
	}
      }
      break;
    default:
      break;
    }
    return 0;
  case 3:
    addr = readbyte(pc++);
    addr = addr << 8;
    addr += readbyte(pc++);
    return addr;
  default:
    return 0;
  }
}

unsigned short getop_short(unsigned char opcode) {
  unsigned short sh, addr;
  if (!(opcode & LAST_QUARTER)) {
    sh = readbyte(pc++);
    sh = sh << 8;
    sh += readbyte(pc++);
  } else {
    addr = getop_addr(opcode);
    sh = readbyte(addr++);
    sh = sh << 8;
    sh += readbyte(addr);
  }
  return sh;
}

unsigned char getop(unsigned char opcode) {
  if (!(opcode & LAST_QUARTER)) {
    return readbyte(pc++);
  }
  return readbyte(getop_addr(opcode));
}

short branch(unsigned char opcode) {
  switch (opcode) {
  case 0:
    break;
  case 2:
    if (FLAG_C | FLAG_Z) return 0;
    break;
  case 3:
    if (!(FLAG_C | FLAG_Z)) return 0;
    break;
  case 4:
    if (FLAG_C) return 0;
    break;
  case 5:
    if (!FLAG_C) return 0;
    break;
  case 6:
    if (FLAG_Z) return 0;
    break;
  case 7:
    if (!FLAG_Z) return 0;
    break;
  case 8:
    if (FLAG_V) return 0;
    break;
  case 9:
    if (!FLAG_V) return 0;
    break;
  case 10:
    if (FLAG_N) return 0;
    break;
  case 11:
    if (!FLAG_N) return 0;
    break;
  case 12:
    if (!(FLAG_N^FLAG_V)) return 0;
    break;
  case 13:
    if (FLAG_N^FLAG_V) return 0;
    break;
  case 14:
    if (!(FLAG_Z | (FLAG_N^FLAG_V))) return 0;
    break;
  case 15:
    if (FLAG_Z | (FLAG_N^FLAG_V)) return 0;
    break;
  default:
    return 0;
  }
  return 1;
}

void jmp(unsigned char opcode) {
  switch (opcode) {
  case 5:
    opcode = 2;
    break;
  case 6:
    opcode = 3;
    break;
  default:
    return;
  }
  pc = getop_short(opcode);
}

void bsr() {
  unsigned short *mem;
  char c;
  c = (signed char)readbyte(pc++);
  sp--;
  sp--;
  mem = (unsigned short*)stackptr(sp);
  *mem = pc;
  pc += c;
}

void jsr(unsigned char opcode) {
  unsigned short operand, *mem;
  switch (opcode) {
  case 7:
    opcode = 1;
    break;
  case 5:
    opcode = 2;
    break;
  case 6:
    opcode = 3;
    break;
  default:
    return;
  }
  operand = getop_addr(opcode);
  if (operand == 0xEE86) {
    putchar(ACCUM_B);
    return;
  }
  if (operand == 0xEE84) {
    ACCUM_B = getchar();
    if (ACCUM_B == 10)
      ACCUM_B = getchar();
    return;
  }
  if (operand == 0xEE88) {
    printf((char*)getptr(d.reg));
    return;
  }
  sp--;
  sp--;
  mem = (unsigned short*)stackptr(sp);
  *mem = pc;
  pc = operand;
}

void lea(unsigned char opcode) {
  unsigned short *reg;
  reg = 0;
  switch (opcode) {
  case 9:
    reg = &y;
    break;
  case 10:
    reg = &x;
    break;
  case 11:
    reg = &sp;
    break;
  default:
    return;
  }
  *reg = getop_short(2);
}

void bset(unsigned char addr_type) {
  unsigned char *mem, mask;
  unsigned short addr;
  addr = getop_addr(addr_type);
  mem = getptr(addr);
  mask = readbyte(pc++);
  *mem = *mem | mask;
}

void bclr(unsigned char addr_type) {
  unsigned char *mem, mask;
  unsigned short addr;
  addr = getop_addr(addr_type);
  mem = getptr(addr);
  mask = readbyte(pc++);
  *mem = *mem & (~mask);
}

void brset(unsigned char addr_type) {
  unsigned char byte, mask;
  char c;
  byte = getop(addr_type);
  mask = readbyte(pc++);
  c = readbyte(pc++);
  if (!((~byte) & mask)) {
    pc += c;
  }
}

void brclr(unsigned char addr_type) {
  unsigned char byte, mask;
  char c;
  byte = getop(addr_type);
  mask = readbyte(pc++);
  c = (signed char)readbyte(pc++);
  if (!(byte & mask)) {
    pc += c;
  }
}

void long_branch(unsigned char opcode) {
  short addr;
  addr = readbyte(pc++);
  addr = addr << 8;
  addr += readbyte(pc++);
  if (branch(opcode)) {
    pc += addr;
  }
}

void short_branch(unsigned char opcode) {
  char c;
  c = (signed char)readbyte(pc++);
  if (branch(opcode)) {
    pc += c;
  }
}

void pshpul(unsigned char opcode) {
  unsigned short *reg;
  unsigned char *addr;
  if (opcode < 8) {
    reg = 0;
    switch (opcode & LAST_QUARTER) {
    case 0:
      reg = &x;
      break;
    case 1:
      reg = &y;
      break;
    case 2:
      if (opcode & THIRD_BIT) {
	sp--;
	addr = stackptr(sp);
	*addr = ACCUM_A;
      } else {
	addr = stackptr(sp);
	ACCUM_A = *addr;
	sp++;
      }
      return;
    case 3:
      if (opcode & THIRD_BIT) {
	sp--;
	addr = stackptr(sp);
	*addr = ACCUM_B;
      } else {
	addr = stackptr(sp);
	ACCUM_B = *addr;
	sp++;
      }
      return;
    default:
      break;
    }
    if (opcode & THIRD_BIT) {
      sp--;
      sp--;
      addr = stackptr(sp);
      *((unsigned short*)addr) = *reg;
    } else {
      addr = stackptr(sp);
      *reg = *((unsigned short*)addr);
      sp++;
      sp++;
    }
  } else if (opcode < 12) {
    if (opcode & SECOND_BIT) {
      reg = &d.reg;
    } else {
      reg = &cc;
    }
    if (opcode & FIRST_BIT) {
      addr = stackptr(sp);
      *reg = *((unsigned short*)addr);
      sp++;
      sp++;
    } else {
      sp--;
      sp--;
      addr = stackptr(sp);
      *((unsigned short*)addr) = *reg;
    }
  } else {
    switch (opcode) {
    case 12:
      /* wavr */
      break;
    case 13:
      addr = stackptr(sp);
      pc = *addr;
      sp++;
      sp++;
      break;
    case 14:
      /* Wait for interrupt */
      break;
    case 15:
      exit(0);
      break;
    default:
      break;
    }
  }
}

void neg(unsigned char opcode) {
  unsigned short addr;
  unsigned char *accum;
  switch (opcode & LAST_QUARTER) {
  case 0:
    accum = &ACCUM_A;
    break;
  case 1:
    accum = &ACCUM_B;
    break;
  default:
    addr = getop_addr(opcode);
    accum = getptr(addr);
    break;
  }
  *accum = (~*accum) + 1;
}

void com(unsigned char opcode) {
  unsigned short addr;
  unsigned char *accum;
  switch (opcode & LAST_QUARTER) {
  case 0:
    accum = &ACCUM_A;
    break;
  case 1:
    accum = &ACCUM_B;
    break;
  default:
    addr = getop_addr(opcode);
    accum = getptr(addr);
    break;
  }
  *accum = ~*accum;
}

void inc(unsigned char opcode) {
  unsigned short addr;
  unsigned char *accum;
  switch (opcode & LAST_QUARTER) {
  case 0:
    accum = &ACCUM_A;
    break;
  case 1:
    accum = &ACCUM_B;
    break;
  default:
    addr = getop_addr(opcode);
    accum = getptr(addr);
    break;
  }
  *accum += 1;
}

void dec(unsigned char opcode) {
  unsigned short addr;
  unsigned char *accum;
  switch (opcode & LAST_QUARTER) {
  case 0:
    accum = &ACCUM_A;
    break;
  case 1:
    accum = &ACCUM_B;
    break;
  default:
    addr = getop_addr(opcode);
    accum = getptr(addr);
    break;
  }
  *accum -= 1;
}

void lsr(unsigned char opcode) {
  unsigned short addr;
  unsigned char *accum;
  switch (opcode & LAST_QUARTER) {
  case 0:
    accum = &ACCUM_A;
    break;
  case 1:
    accum = &ACCUM_B;
    break;
  default:
    addr = getop_addr(opcode);
    accum = getptr(addr);
    break;
  }
  *accum = *accum >> 1;
}

void rol(unsigned char opcode) {
  unsigned short addr;
  unsigned char *accum, bit;
  switch (opcode & LAST_QUARTER) {
  case 0:
    accum = &ACCUM_A;
    break;
  case 1:
    accum = &ACCUM_B;
    break;
  default:
    addr = getop_addr(opcode);
    accum = getptr(addr);
    break;
  }
  bit = FLAG_C;
  if (*accum & 0x80) {
    cc = cc | 0x01;
  }
  *accum = (*accum << 1) + bit;
}

void ror(unsigned char opcode) {
  unsigned short addr;
  unsigned char *accum, bit;
  switch (opcode & LAST_QUARTER) {
  case 0:
    accum = &ACCUM_A;
    break;
  case 1:
    accum = &ACCUM_B;
    break;
  default:
    addr = getop_addr(opcode);
    accum = getptr(addr);
    break;
  }
  bit = FLAG_C;
  if (*accum & 0x01) {
    cc = cc | 0x01;
  }
  *accum = (*accum >> 1) + bit;
}

void asr(unsigned char opcode) {
  unsigned short addr;
  unsigned char *accum;
  switch (opcode & LAST_QUARTER) {
  case 0:
    accum = &ACCUM_A;
    break;
  case 1:
    accum = &ACCUM_B;
    break;
  default:
    addr = getop_addr(opcode);
    accum = getptr(addr);
    break;
  }
  *accum = *((signed char*)accum) >> 1;
}

void asl(unsigned char opcode) {
  unsigned short addr;
  unsigned char *accum;
  switch (opcode & LAST_QUARTER) {
  case 0:
    accum = &ACCUM_A;
    break;
  case 1:
    accum = &ACCUM_B;
    break;
  default:
    addr = getop_addr(opcode);
    accum = getptr(addr);
    break;
  }
  *accum = *accum << 1;
}

void clr(unsigned char opcode) {
  unsigned short addr;
  unsigned char *accum;
  switch (opcode & LAST_QUARTER) {
  case 0:
    d.reg = d.reg >> 1;
    break;
  case 1:
    d.reg = d.reg << 1;
    break;
  default:
    addr = getop_addr(opcode);
    accum = getptr(addr);
    *accum = 0;
  }
}

void staa(unsigned char opcode) {
  if (!(opcode & LAST_QUARTER)) {
    /* Call with extended */
  }
  ACCUM_A = getop(opcode);
}

void stab(unsigned char opcode) {
  if (!(opcode & LAST_QUARTER)) {
    /* Call with indirect */
  }
  ACCUM_B = getop(opcode);
}

void std(unsigned char opcode) {
  unsigned short addr;
  unsigned short* mem;
  if (!(opcode & LAST_QUARTER)) {
    bset(1);
  }
  addr = getop_short(opcode);
  mem = (unsigned short*)getptr(addr);
  *mem = d.reg;
}

void sty(unsigned char opcode) {
  unsigned short addr;
  unsigned short* mem;
  if (!(opcode & LAST_QUARTER)) {
    bclr(1);
  }
  addr = getop_short(opcode);
  mem = (unsigned short*)getptr(addr);
  *mem = y;
}

void stx(unsigned char opcode) {
  unsigned short addr;
  unsigned short* mem;
  if (!(opcode & LAST_QUARTER)) {
    brset(1);
  }
  addr = getop_short(opcode);
  mem = (unsigned short*)getptr(addr);
  *mem = x;
}

void sts(unsigned char opcode) {
  unsigned short addr;
  unsigned short* mem;
  if (!(opcode & LAST_QUARTER)) {
    brclr(1);
  }
  addr = getop_short(opcode);
  mem = (unsigned short*)getptr(addr);
  *mem = sp;
}


void sub_accum(unsigned char opcode) {
  unsigned char operand = getop(opcode & LAST_QUARTER);
  unsigned char *accum;
  char res;
  switch (opcode & THIRD_QUARTER) {
  case 8:
    accum = &ACCUM_A;
    break;
  case 12:
    accum = &ACCUM_B;
    break;
  default:
    return;
  }
  res = *accum - operand;
  if (res < 0) {
    cc |= 0x08;
  } else {
    cc &= 0xF7;
  }
  if (!res) {
    cc |= 0x04;
  } else {
    cc &= 0xFC;
  }
  if ((*accum & MSB_SET    &&   operand & MSB_SET  && res > 0) ||
      (!(*accum & MSB_SET) && !(operand & MSB_SET) && res < 0)) {
    cc |= 0x02;
  } else {
    cc &= 0xFD;
  }
  if (((unsigned char)res) < *accum) {
    cc |= 0x01;
  } else {
    cc &= 0xFE;
  }
  *accum = res;
}

void cmp_accum(unsigned char opcode) {
  unsigned char operand = getop(opcode & LAST_QUARTER);
  unsigned char *accum;
  char res;
  switch (opcode & THIRD_QUARTER) {
  case 8:
    accum = &ACCUM_A;
    break;
  case 12:
    accum = &ACCUM_B;
    break;
  default:
    return;
  }
  res = *accum - operand;
  if (res < 0) {
    cc |= 0x08;
  } else {
    cc &= 0xF7;
  }
  if (!res) {
    cc |= 0x04;
  } else {
    cc &= 0xFC;
  }
  if ((*accum & MSB_SET    &&   operand & MSB_SET  && res > 0) ||
      (!(*accum & MSB_SET) && !(operand & MSB_SET) && res < 0)) {
    cc |= 0x02;
  } else {
    cc &= 0xFD;
  }
  if (((unsigned char)res) < *accum) {
    cc |= 0x01;
  } else {
    cc &= 0xFE;
  }
}

void sbc_accum(unsigned char opcode) {
  unsigned char operand = getop(opcode & LAST_QUARTER);
  unsigned char *accum;
  switch (opcode & THIRD_QUARTER) {
  case 8:
    accum = &ACCUM_A;
    break;
  case 12:
    accum = &ACCUM_B;
    break;
  default:
    return;
  }
  if (FLAG_C) {
    operand++;
  }
  /* flag registers */
  *accum -= operand;
}

void add_d(unsigned char opcode) {
  unsigned short operand = getop_short(opcode & LAST_QUARTER);
  switch (opcode & THIRD_QUARTER) {
  case 8:
    /* SUB */
    break;
  case 12:
    /* ADD */
    break;
  default:
    return;
  }
}

void and_accum(unsigned char opcode) {
  unsigned char operand = getop(opcode & LAST_QUARTER);
  unsigned char *accum;
  switch (opcode & THIRD_QUARTER) {
  case 8:
    accum = &ACCUM_A;
    break;
  case 12:
    accum = &ACCUM_B;
    break;
  default:
    return;
  }
  /* flag registers */
  *accum = operand & *accum;
}

void bit_accum(unsigned char opcode) {
  unsigned char operand = getop(opcode & LAST_QUARTER);
  unsigned char *accum;
  switch (opcode & THIRD_QUARTER) {
  case 8:
    accum = &ACCUM_A;
    break;
  case 12:
    accum = &ACCUM_B;
    break;
  default:
    return;
  }
  /* flag registers for bitwise AND */
}

void load_accum(unsigned char opcode) {
  unsigned char operand = getop(opcode & LAST_QUARTER);
  unsigned char *accum;
  switch (opcode & THIRD_QUARTER) {
  case 8:
    accum = &ACCUM_A;
    break;
  case 12:
    accum = &ACCUM_B;
    break;
  default:
    return;
  }
  *accum = operand;
}

void clr_tst_accum(unsigned char opcode) {
  unsigned short* reg = 0, sh;
  unsigned char* accum = 0;
  unsigned char operand, c;
  if (opcode & SECOND_BIT) {
    if (opcode & THIRD_BIT) {
      operand = getop(opcode);
      /* set flags register per operand value */
    } else {
      if (opcode & FIRST_BIT) {
	operand = readbyte(pc++);
	switch (operand & SECOND_HALF) {
	case 0:
	  accum = &ACCUM_A;
	  break;
	case 1:
	  accum = &ACCUM_B;
	  break;
	case 2:
	  reg = &cc;
	  break;
	case 4:
	  reg = &d.reg;
	  break;
	case 5:
	  reg = &x;
	  break;
	case 6:
	  reg = &y;
	  break;
	case 7:
	  reg = &sp;
	  break;
	default:
	  return;
	}
	switch ((operand & FIRST_HALF) >> 4) {
	case 0:
	  if (reg) {
	    *reg = ACCUM_A;
	  } else {
	    *accum = ACCUM_A;
	  }
	  break;
	case 1:
	  if (reg) {
	    *reg = ACCUM_B;
	  } else {
	    *accum = ACCUM_B;
	  }
	  break;
	case 2:
	  if (reg) {
	    *reg = cc;
	  } else {
	    *accum = *(((unsigned char*)&cc) + 1);
	  }
	  break;
	case 4:
	  if (reg) {
	    *reg = d.reg;
	  } else {
	    *accum = ACCUM_B;
	  }
	  break;
	case 5:
	  if (reg) {
	    *reg = x;
	  } else {
	    *accum = *(((unsigned char*)&x) + 1);
	  }
	  break;
	case 6:
	  if (reg) {
	    *reg = y;
	  } else {
	    *accum = *(((unsigned char*)&y) + 1);
	  }
	  break;
	case 7:
	  if (reg) {
	    *reg = sp;
	  } else {
	    *accum = *(((unsigned char*)&sp) + 1);
	  }
	  break;
	case 8:
	  if (reg) {
	    sh = *reg;
	    *reg = ACCUM_A;
	    ACCUM_A = *(((unsigned char*)&sh) + 1);
	  } else {
	    c = *accum;
	    *accum = ACCUM_A;
	    ACCUM_A = c;
	  }
	  break;
	case 9:
	  if (reg) {
	    sh = *reg;
	    *reg = ACCUM_B;
	    ACCUM_B = *(((unsigned char*)&sh) + 1);
	  } else {
	    c = *accum;
	    *accum = ACCUM_B;
	    ACCUM_B = c;
	  }
	  break;
	case 10:
	  if (reg) {
	    sh = *reg;
	    *reg = cc;
	    cc = *(((unsigned char*)&sh) + 1);
	  } else {
	    c = *accum;
	    *accum = cc;
	    cc = c;
	  }
	  break;
	case 12:
	  if (reg) {
	    sh = *reg;
	    *reg = d.reg;
	    d.reg = sh;
	  } else if (accum == &ACCUM_B) {
	    ACCUM_A = 0xFF;
	  } else {
	    c = ACCUM_B;
	    ACCUM_B = ACCUM_A;
	    ACCUM_A = c;
	  }
	  break;
	case 13:
	  if (reg) {
	    sh = *reg;
	    *reg = x;
	    x = sh;
	  } else if (accum == &ACCUM_B) {
	    c = ACCUM_B;
	    ACCUM_B = *(((unsigned char*)&x) + 1);
	    x = 0xFF;
	    x = x << 8;
	    x += c;
	  } else {
	    c = ACCUM_A;
	    x = 0x00;
	    ACCUM_A = *(((unsigned char*)&x) + 1);
	    x = 0x00;
	    x = x << 8;
	    x += c;
	  }
	  break;
	case 14:
	  if (reg) {
	    sh = *reg;
	    *reg = y;
	    y = sh;
	  } else if (accum == &ACCUM_B) {
	    c = ACCUM_B;
	    ACCUM_B = *(((unsigned char*)&y) + 1);
	    y = 0xFF;
	    y = y << 8;
	    y += c;
	  } else {
	    c = ACCUM_A;
	    y = 0x00;
	    ACCUM_A = *(((unsigned char*)&y) + 1);
	    y = 0x00;
	    y = y << 8;
	    y += c;
	  }
	  break;
	case 15:
	  if (reg) {
	    sh = *reg;
	    *reg = sp;
	    sp = sh;
	  } else if (accum == &ACCUM_B) {
	    c = ACCUM_B;
	    ACCUM_B = *(((unsigned char*)&sp) + 1);
	    sp = 0xFF;
	    sp = sp << 8;
	    sp += c;
	  } else {
	    c = ACCUM_A;
	    sp = 0x00;
	    ACCUM_A = *(((unsigned char*)&sp) + 1);
	    sp = 0x00;
	    sp = sp << 8;
	    sp += c;
	  }
	  break;
	default:
	  return;
	}
      } else {
	return; /* NOP instruction */
      }
    }
  } else {
    if (opcode & THIRD_BIT) {
      accum = &ACCUM_B;
    } else {
      accum = &ACCUM_A;
    }
    if (opcode & FIRST_BIT) {
      /* set flags register per *accum value */
    } else {
      *accum = 0;
    }
  }
}

void xor_accum(unsigned char opcode) {
  unsigned char operand = getop(opcode & LAST_QUARTER);
  unsigned char *accum;
  switch (opcode & THIRD_QUARTER) {
  case 8:
    accum = &ACCUM_A;
    break;
  case 12:
    accum = &ACCUM_B;
    break;
  default:
    return;
  }
  /* flag registers */
  *accum = operand ^ *accum;
}

void adc_accum(unsigned char opcode) {
  unsigned char operand = getop(opcode & LAST_QUARTER);
  unsigned char *accum;
  switch (opcode & THIRD_QUARTER) {
  case 8:
    accum = &ACCUM_A;
    break;
  case 12:
    accum = &ACCUM_B;
    break;
  default:
    return;
  }
  if (FLAG_C) {
    operand++;
  }
  /* flag registers */
  *accum += operand;
}

void or_accum(unsigned char opcode) {
  unsigned char operand = getop(opcode & LAST_QUARTER);
  unsigned char *accum;
  switch (opcode & THIRD_QUARTER) {
  case 8:
    accum = &ACCUM_A;
    break;
  case 12:
    accum = &ACCUM_B;
    break;
  default:
    return;
  }
  /* flag registers */
  *accum = operand | *accum;
}

void add_accum(unsigned char opcode) {
  unsigned char operand = getop(opcode & LAST_QUARTER);
  unsigned char *accum;
  switch (opcode & THIRD_QUARTER) {
  case 8:
    accum = &ACCUM_A;
    break;
  case 12:
    accum = &ACCUM_B;
    break;
  default:
    return;
  }
  /* flag registers */
  *accum += operand;
}

void cp_ld_d(unsigned char opcode) {
  unsigned short operand = getop_short(opcode & LAST_QUARTER);
  switch (opcode & THIRD_QUARTER) {
  case 8:
    /* flag register for cmp */
    /* d.reg - operand */
    break;
  case 12:
    d.reg = operand;
    break;
  default:
    return;
  }
}

void cp_ld_y(unsigned char opcode) {
  unsigned short operand = getop_short(opcode & LAST_QUARTER);
  switch (opcode & THIRD_QUARTER) {
  case 8:
    /* flag register for cmp */
    /* y - operand */
    break;
  case 12:
    y = operand;
    break;
  default:
    return;
  }
}

void cp_ld_x(unsigned char opcode) {
  unsigned short operand = getop_short(opcode & LAST_QUARTER);
  switch (opcode & THIRD_QUARTER) {
  case 8:
    /* flag register for cmp */
    /* x - operand */
    break;
  case 12:
    x = operand;
    break;
  default:
    return;
  }
}

void cp_ld_sp(unsigned char opcode) {
  unsigned short operand = getop_short(opcode & LAST_QUARTER);
  switch (opcode & THIRD_QUARTER) {
  case 8:
    /* flag register for cmp */
    /* sp - operand */
    break;
  case 12:
    sp = operand;
    break;
  default:
    return;
  }
}

void (*msb_opcode_array[16])(unsigned char opcode) = { &sub_accum, &cmp_accum, &sbc_accum, &add_d, &and_accum, &bit_accum, &load_accum, &clr_tst_accum, &xor_accum, &adc_accum, &or_accum, &add_accum, &cp_ld_d, &cp_ld_y, &cp_ld_x, &cp_ld_sp };
void (*smsb_opcode_array[16])(unsigned char opcode) = { &neg, &com, &inc, &dec, &lsr, &rol, &ror, &asr, &asl, &clr, &staa, &stab, &std, &sty, &stx, &sts};
void (*branch_clr_set[4])(unsigned char addr_type) = { &bset, &bclr, &brset, &brclr };
