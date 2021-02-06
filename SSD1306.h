#ifndef SSD1306
#define SSD1306 0.1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include "linux/i2c-dev.h"
#include "linux/i2c.h"
//#include "i2c/smbus.h"

#include "i2c.h"

// Constants

#define SSD1306_ADDR_PRIM 0x3C
#define SSD1306_ADDR_SECO 0x3D

#define SSD1306_CTRL_COMM 0x00
#define SSD1306_CTRL_DATA 0x40

// Commands
//power
#define DISPLAYOFF 0xAE
#define DISPLAYON 0xAF
#define DISPLAYRESUME 0xA4
#define DISPLAYALLON 0xA5
//screenconfig
#define SETCONTRAST 0x81
#define COLORNORMAL 0xA6
#define COLORINVERT 0xA7

// Constants

#define SETDISPLAYOFFSET 0xD3
#define SETCOMPINS 0xDA
#define SETVCOMDETECT 0xDB
#define SETDISPLAYCLOCKDIV 0xD5
#define SETPRECHARGE 0xD9
#define SETMULTIPLEX 0xA8
#define SETLOWCOLUMN 0x00
#define SETHIGHCOLUMN 0x10
#define SETSTARTLINE 0x40
#define MEMORYMODE 0x20
#define COLUMNADDR 0x21
#define PAGEADDR 0x22
#define COMSCANINC 0xC0
#define COMSCANDEC 0xC8
#define SEGREMAP 0xA0
#define CHARGEPUMP 0x8D
#define EXTERNALVCC 0x1
#define SWITCHCAPVCC 0x2

// Scrolling constants
#define ACTIVATE_SCROLL 0x2F
#define DEACTIVATE_SCROLL 0x2E
#define SET_VERTICAL_SCROLL_AREA 0xA3
#define RIGHT_HORIZONTAL_SCROLL 0x26
#define LEFT_HORIZONTAL_SCROLL 0x27
#define VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A

// Array of bitset constants
const uint8_t bitset[] = {1,2,4,8,16,32,64,128};

// Defines a display instance. Underscore indicates that the variable should usually not be modified once initialized.
struct SSD1306_dispconf{
	uint8_t _width;
	uint8_t _height;
	uint8_t _pages;
	int _bufferlen;
	int _canvaslen;
	uint8_t _i2c_dev_fd;
	uint8_t _vccstate;
	uint8_t contrast;
	uint8_t *buffer;
};

int SSD1306_ctrl(struct SSD1306_dispconf *disp,uint8_t command);

int SSD1306_data_byte(struct SSD1306_dispconf *disp,uint8_t data);

int SSD1306_buffer_flush(struct SSD1306_dispconf *disp);

int SSD1306_buffer_from_1darray(struct SSD1306_dispconf *disp,uint8_t *canvas);

void SSD1306_destroy(struct SSD1306_dispconf *disp);

#endif