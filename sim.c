#include "consts.h"
#include "helper.h"
#include "objgen.h"
#include "opcode.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <winsock.h>
#else
#include <arpa/inet.h>
#endif

void set_status(signed short res, unsigned short carries, unsigned char flags);

union reg_accum d;
unsigned short pc = 0, x = 0, y = 0, sp = 0, cc = 0;
unsigned char ppage;

extern List* seglist;
extern void (*msb_opcode_array[16])(unsigned char opcode);
extern void (*smsb_opcode_array[16])(unsigned char opcode);
extern void (*branch_clr_set[4])(unsigned char addr_type);

void execute(void) {
  unsigned char c;
  unsigned char *mem;
  for (;;) {
    c = readbyte(pc++);
    if (c == 0x18) { /* second byte */
      c = readbyte(pc++);
      if ((c & FIRST_HALF) == BRANCH) {
	long_branch(c & SECOND_HALF);
      } else if (!(c & FIRST_HALF)) {
	mov(c & SECOND_HALF);
      } else if ((c & FIRST_HALF) == FIFTH_BIT) {
	divmul(c & SECOND_HALF);
      } else {
	printf("Unimplemented second byte: ");
	printbyte(c);
	printf("\n");
	return;
      }
    } else if (c == 0x04) { /* loop instruction */
      unsigned char lb = readbyte(pc++);
      unsigned char rr = readbyte(pc++);
      char branch;
      char invert = !(lb & 0x20); /* 0 if BNE, 1 if BEQ */
      signed char offset = 0;
      switch (lb & 0xC0) {
      case 0x00: /* D */
	offset = -1;
	break;
      case 0x40: /* I */
	offset = +1;
	break;
      case 0x80: /* T */
	offset = 0;
	break;
      default:
	printf("Unimplemented loop type\n");
	return;
      }
      if (lb & 0x04) {
	/* 16-bit counter */
	unsigned short *counter;
	switch (lb & 3) {
	case 0:
	  counter = &d.reg;
	  break;
	case 1:
	  counter = &x;
	  break;
	case 2:
	  counter = &y;
	  break;
	default:
	  counter = &sp;
	  break;
	}
	*counter += offset;
	branch = invert ^ (*counter != 0);
      } else {
	/* 8-bit counter */
	unsigned char *counter;
	switch (lb & 3) {
	case 0:
	  counter = &ACCUM_A;
	  break;
	case 1:
	  counter = &ACCUM_B;
	  break;
	default:
	  printf("Unimplemented loop counter\n");
	  return;
	}
	*counter += offset;
	branch = invert ^ (*counter != 0);
      }
      if (branch) {
	pc += rr;
	if (lb & 0x10)
	  pc -= 256;
      }
    } else if (c & MSB_SET) {
      msb_opcode_array[c & SECOND_HALF]((c & FIRST_HALF) >> 4);
    } else if (c & SMSB_SET) {
      smsb_opcode_array[c & SECOND_HALF]((c & FIRST_HALF) >> 4);
    } else if ((c & FIRST_HALF) == BRANCH) {
      short_branch(c & SECOND_HALF);
    } else if ((c & FIRST_HALF) == PSHPUL) {
      pshpul(c & SECOND_HALF);
    } else if ((c & SECOND_HALF) > 11) {
      branch_clr_set[(c & SECOND_HALF) - 12](((c & FIRST_HALF) >> 4) + 2);
    } else {
      switch (c) {
      case 0x00:
	/* bgnd */
	break;
      case 0x01:
	/* mem */
	break;
      case 0x02:
	y++;
	set_status(y, 0, 0x04);
	break;
      case 0x03:
	y--;
	set_status(y, 0, 0x04);
	break;
      case 0x05:
      case 0x06:
	jmp(c);
	break;
      case 0x07:
	bsr();
	break;
      case 0x08:
	x++;
	set_status(x, 0, 0x04);
	break;
      case 0x09:
	x--;
	set_status(x, 0, 0x04);
	break;
      case 0x0A:
	mem = stackptr(sp);
	ppage = *mem;
	sp++;
	mem = stackptr(sp);
	pc = ntohs(*((unsigned short*)mem));
	sp++;
	sp++;
	break;
      case 0x0B:
	/* Return from interrupt */
	break;
      case 0x10:
	c = readbyte(pc++);
	cc = cc & c;
	break;
      case 0x11: /* EDIV */
	if (x == 0) {
	  cc |= 0x01;
	} else {
	  uint32_t numerator = (y << 16) + d.reg;
	  uint32_t quotient = numerator / x;
	  y = quotient;
	  d.reg = numerator % x;
	  set_status(y, 0, 0x0F);
	  if (quotient >= 0x80000)
	    cc |= 0x02;
	}
	break;
      case 0x12: /* MUL */
	d.reg = ACCUM_A * ACCUM_B;
	set_status(0, d.reg << 1, 0x01);
	break;
      case 0x13: /* EMUL */
      {
	uint32_t res = d.reg * y;
	y = res >> 16;
	d.reg = res & 0xFFFFu;
	set_status(res | (res >> 8) | (res >> 16), res >> 7, 0x0D);
	break;
      }
      case 0x14:
	c = readbyte(pc++);
	cc = cc | c;
	break;
      case 0x15:
      case 0x16:
      case 0x17:
	jsr(c & SECOND_HALF);
	break;
      case 0x19:
      case 0x1A:
      case 0x1B:
	lea(c & SECOND_HALF);
	break;
      default:
	printf("Unimplemented opcode: ");
	printbyte(c);
	printf("\n");
	return;
      }
    }
  }
}

int main(int argc, char** argv) {
  List* freelist;
  FILE* srecord;
  if (argc < 2)
    return 1;
  seglist = newList();
  srecord = fopen(argv[1], "r");
  pc = load_srecord(srecord, seglist);
  if (!pc)
    pc = 0x0400;
  d.reg = 0;
  execute();
  for (freelist = seglist;freelist != NULL; freelist = freelist->next) {
    free(freelist->data);
  }
  free(seglist);
  return 0;
}
