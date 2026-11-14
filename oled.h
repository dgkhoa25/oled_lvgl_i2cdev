#ifndef OLED_H
#define OLED_H

#include <stdint.h>

/* Chinh cho dung voi code u8g2 cua ban */
#define OLED_I2C_DEV   "/dev/i2c-2"   /* BBB: I2C2 (P9_19/P9_20) thuong la i2c-2 */
#define OLED_I2C_ADDR  0x3C

#define OLED_W   128
#define OLED_H   64

/* Dat 1 neu hinh bi xoay 180 do (dao SEG va COM) */
#define OLED_FLIP  0

int  oled_init(void);                                   /* 0 = OK */
void oled_write_page(uint8_t page, uint8_t col,
                     const uint8_t *data, int len);     /* ghi 1 page (8 hang) */
void oled_close(void);

#endif /* OLED_H */