#include "simple_uuid.h"
#include <esp_random.h>
#include <stdint.h>
void simple_uuid(char uuid_str[37]) {
  uint32_t ar[4];
  for (uint8_t i = 0; i < 4; i++) {
    ar[i] = esp_random();
    //  store binary version globally ?
    //  _ar[i] = ar[i];
  }
  //   esp_fill_random(ar,4);
  //  Conforming to RFC 4122 Specification
  //  - byte 7: four most significant bits ==> 0100  --> always 4
  //  - byte 9: two  most significant bits ==> 10    --> always {8, 9, A, B}.
  //
  //  patch bits for version 1 and variant 4 here
  //   if (_mode == UUID_MODE_VARIANT4) {
  ar[1] &= 0xFFF0FFFF; //  remove 4 bits.
  ar[1] |= 0x00040000; //  variant 4
  ar[2] &= 0xFFFFFFF3; //  remove 2 bits
  ar[2] |= 0x00000008; //  version 1
                       //   }

  //  process 16 bytes build up the char array.
  for (uint8_t i = 0, j = 0; i < 16; i++) {
    //  multiples of 4 between 8 and 20 get a -.
    //  note we are processing 2 digits in one loop.
    if ((i & 0x1) == 0) {
      if ((4 <= i) && (i <= 10)) {
        uuid_str[j++] = '-';
      }
    }

    //  process one byte at the time instead of a nibble
    uint8_t nr = i / 4;
    uint8_t xx = ar[nr];
    uint8_t ch = xx & 0x0F;
    uuid_str[j++] = (ch < 10) ? '0' + ch : ('a' - 10) + ch;

    ch = (xx >> 4) & 0x0F;
    ar[nr] >>= 8;
    uuid_str[j++] = (ch < 10) ? '0' + ch : ('a' - 10) + ch;
  }

  //  if (_upperCase)
  //  {
  //    for (int i = 0; i < 37; i++)
  //    {
  //      _buffer[i] = toUpper(_buffer[i]);
  //    }
  //  }
  uuid_str[36] = 0;
}

