#include "list.h"
#include "objgen.h"
#include <stdio.h>
#include <stdlib.h>

List* seglist;
extern unsigned short pc;

void printbyte(char bytebuf) {
  char c, d;
  c = bytebuf >> 4;
  if (c < 0) {
    c += 16;
  }
  d = bytebuf%16;
  if (d < 0) {
    d += 16;
  }
  if (c > 9) c += 7;
  if (d > 9) d += 7;
  printf("%c%c", c+48, d+48);
  return;
}

unsigned char readbyte(unsigned short addr) {
  List* l;
  Segment* s;
  if (0xEE83 < addr && addr < 0xEE8A) {
    return addr%2 == 0 ? 0xEE : (((unsigned char)addr) - 1);
  }
  for (l = seglist;l != NULL;l = l->next) {
    s = l->data;
    if (s->addr <= addr && addr <= (s->addr + s->len)) {
      /* if (addr+1 == pc) { */
      /* 	printbyte(s->buf[addr - s->addr]); */
      /* } */
      return s->buf[addr - s->addr];
    }
  }
  return 0;
}

unsigned char* getptr(unsigned short addr) {
  List* l;
  Segment* s;
  for (l = seglist;l != NULL;l = l->next) {
    s = l->data;
    if (s->addr <= addr && addr <= (s->addr + s->len)) {
      return &(s->buf[addr - s->addr]);
    }
  }
  return 0;
}

unsigned char* stackptr(unsigned short sp) {
  unsigned char* ptr;
  Segment* s;
  ptr = getptr(sp);
  if (!ptr) {
    s = malloc(sizeof(Segment));
    if (sp < 1024) {
      s->len = sp;
      s->addr = 0;
    } else {
      s->len = 1024;
      s->addr = sp - s->len + 1;
    }
    s->buf = malloc(s->len + 1);
    s->buf[s->len] = '\0';
    addToListEnd(seglist, s);
    ptr = getptr(sp);
  }
  return ptr;
}
