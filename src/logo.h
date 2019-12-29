#include <avr/pgmspace.h>

// logo parameters
#define snake_logo_h 27
#define snake_logo_w 71

const unsigned char PROGMEM logo_snake[] = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
  ,0x01,0xff,0x80,0x00,0x00,0x00,0x00,0x00,0x00
  ,0x07,0x00,0xe0,0x00,0x00,0x00,0x00,0x00,0x00
  ,0x0c,0x00,0x30,0x00,0x00,0x7c,0x00,0x00,0x00
  ,0x08,0x00,0x10,0x00,0x00,0xc6,0x00,0x00,0x00
  ,0x08,0x7c,0x50,0x00,0x01,0x82,0x1e,0x00,0x00
  ,0x08,0xc7,0x10,0x00,0x01,0x02,0x33,0x00,0x00
  ,0x08,0x81,0xf8,0x00,0x01,0xe3,0x61,0x80,0x00
  ,0x08,0x80,0x0c,0x00,0x00,0x31,0x40,0x9f,0xe0
  ,0x08,0xe0,0x00,0x00,0xff,0x11,0xc0,0xb0,0x30
  ,0x08,0x38,0x3e,0x00,0x81,0x99,0x8e,0xb0,0x18
  ,0x08,0x0e,0x23,0xf0,0x80,0x89,0x8b,0xa0,0x08
  ,0x0c,0x02,0x22,0x11,0x80,0xcd,0x18,0x23,0x88
  ,0x06,0x03,0x20,0x19,0x0c,0x65,0x10,0x66,0x88
  ,0x03,0xc1,0x20,0x0b,0x3e,0x24,0x18,0x45,0x88
  ,0x00,0x61,0x60,0x0a,0x23,0x2c,0x08,0x47,0x08
  ,0x00,0x31,0x40,0x8a,0x3d,0x28,0x0c,0x40,0x18
  ,0x00,0x31,0x41,0x8a,0x07,0x28,0x06,0x43,0xf0
  ,0x00,0x61,0x43,0x8a,0x00,0x28,0x62,0x46,0x00
  ,0x00,0xc1,0x42,0x8a,0x3c,0x2c,0x63,0x43,0xe0
  ,0x1f,0x83,0x46,0x8a,0x24,0x24,0x71,0x40,0x38
  ,0x30,0x02,0x44,0x8b,0x24,0x24,0xd1,0x60,0x0c
  ,0x60,0x0e,0x44,0x89,0x26,0x24,0x99,0x30,0x04
  ,0x40,0x18,0x7c,0xc9,0xe2,0x24,0x89,0x1f,0xfc
  ,0x47,0xf0,0x00,0x48,0x03,0x27,0x8d,0x00,0x00
  ,0x7c,0x00,0x00,0x68,0x01,0xe0,0x07,0x00,0x00
  ,0x00,0x00,0x00,0x38,0x00,0x00,0x00,0x00,0x00

};

// logo parameters
#define gameover_logo_h 30
#define gameover_logo_w 96

const unsigned char PROGMEM logo_gameOver[] = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
  ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
  ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
  ,0x3f,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
  ,0x60,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
  ,0x40,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
  ,0x40,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
  ,0x46,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
  ,0x47,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
  ,0x45,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
  ,0x47,0x88,0x00,0x00,0x00,0x00,0x03,0xf8,0x00,0x00,0x00,0x00
  ,0x42,0xc8,0x00,0x00,0x00,0x00,0x0e,0x08,0x00,0x00,0x00,0x00
  ,0x42,0x49,0xf8,0x00,0x00,0xf8,0x38,0x0f,0x9f,0x80,0x00,0x00
  ,0x62,0x49,0x0d,0xf8,0x03,0x8c,0x20,0x08,0x90,0xbf,0x1f,0x7c
  ,0x22,0x7b,0x07,0x08,0x06,0x06,0x60,0x08,0xf0,0xe1,0x91,0xc6
  ,0x23,0xe2,0x03,0x0f,0xfc,0x02,0x41,0x88,0x60,0xc0,0xd1,0x02
  ,0x22,0x3a,0x03,0x01,0x0c,0x02,0x43,0x88,0x61,0x80,0x50,0x02
  ,0x22,0x0e,0x23,0x80,0x04,0x62,0x46,0x8c,0x43,0x8c,0x50,0x02
  ,0x23,0xcc,0x21,0x80,0x04,0x62,0x45,0x8c,0x47,0x0c,0x58,0x3e
  ,0x22,0xcc,0x21,0x80,0x00,0x26,0x45,0x1c,0x05,0x04,0x48,0x60
  ,0x23,0x8c,0x71,0x84,0x20,0x0c,0x47,0x14,0x05,0x00,0xc8,0x40
  ,0x23,0x0c,0x01,0x8c,0x60,0x18,0x46,0x34,0x0d,0x01,0x88,0x40
  ,0x20,0x08,0x01,0x8c,0x62,0x10,0x40,0x26,0x09,0x07,0xc8,0x40
  ,0x20,0x18,0x01,0x8c,0x62,0x1e,0x40,0x62,0x19,0x80,0x68,0x40
  ,0x20,0x30,0xf1,0x8c,0x63,0x03,0x60,0x42,0x10,0x80,0x28,0x40
  ,0x20,0xf0,0x91,0x8c,0x63,0x01,0x20,0xc2,0x10,0xc0,0x28,0x40
  ,0x3f,0x91,0x91,0x8c,0x63,0x81,0x3f,0x83,0x10,0x7c,0x2f,0x40
  ,0x00,0x1f,0x19,0xff,0xfe,0xff,0x00,0x01,0xf0,0x07,0xe1,0xc0
  ,0x00,0x00,0x0f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
  ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};