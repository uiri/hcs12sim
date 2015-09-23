#include "objgen.h"
#include <stdio.h>
#include <stdlib.h>

int getbyte(FILE* srecord) {
  int tmpbyte, databyte;
  tmpbyte = fgetc(srecord);
  if (tmpbyte <= '9')
    tmpbyte -= '0';
  else
    tmpbyte -= '7';
  databyte = tmpbyte * 16;
  tmpbyte = fgetc(srecord);
  if (tmpbyte <= '9')
    tmpbyte -= '0';
  else
    tmpbyte -= '7';
  databyte += tmpbyte;
  return databyte;
}

int load_srecord(FILE* srecord, List* seglist) {
  char c, len;
  short flag = 0;
  int addr, oldaddr = 0;
  Segment* seg = NULL;
  unsigned char* data;
  while (1) {
    addr = 0;
    c = fgetc(srecord); /* S */
    c = fgetc(srecord);
    len = getbyte(srecord);
    len--;
    switch (c) {
    case '3':
      addr += getbyte(srecord);
      len--;
      addr *= 256;
    case '2':
      addr += getbyte(srecord);
      len--;
      addr *= 256;
    case '1':
      addr += getbyte(srecord);
      len--;
      addr *= 256;
      addr += getbyte(srecord);
      len--;
      if (seg == NULL || oldaddr+seg->len != addr) {
	seg = malloc(sizeof(Segment));
	seg->addr = addr;
	seg->len = len;
	data = malloc(len + 1);
	seg->buf = data;
	for (;len>0;len--) {
	  *data = getbyte(srecord);
	  data++;
	}
	*data = '\0';
	addToListEnd(seglist, seg);
	oldaddr = addr;
      } else {
      	data = realloc(seg->buf, seg->len + len + 1);
      	seg->buf = data;
      	data += seg->len;
      	seg->len += len;
      	for (;len>0;len--) {
      	  *data = getbyte(srecord);
      	  data++;
      	}
      	*data = '\0';
      }
      break;
    case '7':
      addr += getbyte(srecord);
      addr *= 256;
    case '8':
      addr += getbyte(srecord);
      addr *= 256;
    case '9':
      addr += getbyte(srecord);
      addr *= 256;
      addr += getbyte(srecord);
      flag = 1;
      break;
    default:
      break;
    }
    while ((c = fgetc(srecord)) != '\n');
    if (flag)
      break;
  }
  return addr;
}
