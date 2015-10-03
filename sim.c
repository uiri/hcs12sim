#include "consts.h"
#include "helper.h"
#include "objgen.h"
#include "opcode.h"
#include <stdio.h>
#include <stdlib.h>

union reg_accum d;
unsigned short pc = 0, x = 0, y = 0, sp = 0, cc = 0;
unsigned char ppage;

extern List* seglist;
extern void (*msb_opcode_array[16])(unsigned char opcode);
extern void (*smsb_opcode_array[16])(unsigned char opcode);
extern void (*branch_clr_set[4])(unsigned char addr_type);

void execute(void) {
  unsigned char c;
  unsigned int i;
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
      printf("loop\n");
      c = readbyte(pc++);
      /* decompose into instruction */
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
	break;
      case 0x03:
	y--;
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
	break;
      case 0x09:
	x--;
	break;
      case 0x0A:
	mem = stackptr(sp);
	ppage = *mem;
	sp++;
	mem = stackptr(sp);
	pc = *((unsigned short*)mem);
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
      case 0x11:
	i = y;
	i = i << 16;
	i += d.reg;
	y = i/x;
	d.reg = i%x;
	break;
      case 0x12:
	ACCUM_A = ACCUM_A * ACCUM_B;
	break;
      case 0x13:
	y = d.reg * y;
	break;
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
