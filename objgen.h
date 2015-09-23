#include "list.h"
#include <stdio.h>

#ifndef OBJGEN
#define OBJGEN

typedef struct segment_record Segment;
struct segment_record {
  int addr;
  int len;
  unsigned char* buf;
};

#endif

int load_srecord(FILE* srecord, List* seglist);
