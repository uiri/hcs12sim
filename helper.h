#ifndef HELPER
#define HELPER

union reg_accum {
  unsigned short reg;
  unsigned char accum[2];
};

#endif

unsigned char* getptr(short addr);
unsigned char readbyte(short addr);
void printbyte(char bytebuf);
unsigned char* stackptr();
