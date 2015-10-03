#ifndef CONSTANTS
#define CONSTANTS

#define ACCUM_A d.accum[1]
#define ACCUM_B d.accum[0]

#define MSB_SET 0x80
#define SMSB_SET 0x40
#define PSHPUL 0x30
#define BRANCH 0x20
#define FIRST_HALF 0xF0
#define SECOND_HALF 0x0F
#define THIRD_QUARTER 0x0C
#define LAST_QUARTER 0x03
#define FIRST_BIT 0x01
#define SECOND_BIT 0x02
#define THIRD_BIT 0x04
#define FOURTH_BIT 0x08
#define FIFTH_BIT 0x10

#define FLAG_S (0x80 & cc)
#define FLAG_X (0x40 & cc)
#define FLAG_H (0x20 & cc)
#define FLAG_I (0x10 & cc)
#define FLAG_N (0x08 & cc)
#define FLAG_Z (0x04 & cc)
#define FLAG_V (0x02 & cc)
#define FLAG_C (0x01 & cc)

#endif
