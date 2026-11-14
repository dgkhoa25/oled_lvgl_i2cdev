/*
 * Driver SSD1306 128x64 qua I2C (user space, /dev/i2c-N), che do page addressing.
 * Cach lam giong u8g2: tu gui chuoi khoi tao, khong dung /dev/fb0.
 */
#include "oled.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#ifndef I2C_SLAVE_FORCE
#define I2C_SLAVE_FORCE 0x0706
#endif

static int fd = -1;

static int i2c_write(const uint8_t *buf, int len)
{
    return (write(fd, buf, len) == len) ? 0 : -1;
}

static int oled_cmds(const uint8_t *cmds, int n)
{
    uint8_t buf[32];
    if (n > (int)sizeof(buf) - 1) return -1;
    buf[0] = 0x00;                       /* Co = 0, D/C# = 0 : lenh */
    memcpy(&buf[1], cmds, n);
    return i2c_write(buf, n + 1);
}

int oled_init(void)
{
    fd = open(OLED_I2C_DEV, O_RDWR);
    if (fd < 0) { perror("open " OLED_I2C_DEV); return -1; }

    /* FORCE: van mo duoc neu kernel driver ssd1307fb dang giu dia chi nay */
    if (ioctl(fd, I2C_SLAVE_FORCE, OLED_I2C_ADDR) < 0) {
        perror("ioctl I2C_SLAVE_FORCE");
        return -1;
    }

    static const uint8_t init_seq[] = {
        0xAE,             /* display off                         */
        0xD5, 0x80,       /* clock divide / oscillator           */
        0xA8, 0x3F,       /* multiplex = 64                      */
        0xD3, 0x00,       /* display offset = 0                  */
        0x40,             /* start line = 0                      */
        0x8D, 0x14,       /* bat charge pump                     */
        0x20, 0x02,       /* PAGE addressing mode                */
#if OLED_FLIP
        0xA0, 0xC0,       /* xoay 180 do                         */
#else
        0xA1, 0xC8,       /* SEG remap, COM scan dec (binh thuong)*/
#endif
        0xDA, 0x12,       /* COM pins: alternative (128x64)      */
        0x81, 0xCF,       /* contrast                            */
        0xD9, 0xF1,       /* pre-charge                          */
        0xDB, 0x40,       /* VCOMH                               */
        0xA4,             /* hien thi theo RAM                   */
        0xA6,             /* khong dao mau                       */
        0xAF              /* display on                          */
    };
    if (oled_cmds(init_seq, sizeof(init_seq)) < 0) {
        perror("oled init");
        return -1;
    }
    return 0;
}

void oled_write_page(uint8_t page, uint8_t col, const uint8_t *data, int len)
{
    uint8_t pos[3] = {
        (uint8_t)(0xB0 | (page & 0x07)),
        (uint8_t)(0x00 | (col & 0x0F)),         /* cot: 4 bit thap  */
        (uint8_t)(0x10 | ((col >> 4) & 0x0F))   /* cot: 4 bit cao   */
    };
    oled_cmds(pos, 3);

    uint8_t buf[1 + OLED_W];
    if (len > OLED_W) len = OLED_W;
    buf[0] = 0x40;                       /* D/C# = 1 : du lieu */
    memcpy(&buf[1], data, len);
    i2c_write(buf, len + 1);
}

void oled_close(void)
{
    if (fd >= 0) {
        static const uint8_t off[] = { 0xAE };
        oled_cmds(off, 1);
        close(fd);
        fd = -1;
    }
}