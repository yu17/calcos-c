#include "SSD1306.h"

const uint8_t bitset[8] = {1,2,4,8,16,32,64,128};

int SSD1306Ctrl(struct SSD1306_dispconf *disp,uint8_t command){
	int ret = i2c_smbus_write_byte_data(disp->_i2c_dev_fd, SSD1306_CTRL_COMM, command);
	if (ret<0) {
		printf("Failed to communicate command 0X%X with errno %d.\n",command,errno);
		exit(1);
	}
	return ret;
}

int SSD1306DataByte(struct SSD1306_dispconf *disp,uint8_t data){
	return i2c_smbus_write_byte_data(disp->_i2c_dev_fd, SSD1306_CTRL_DATA, data);
}

// SSD1306 does not adopt standard block data writing, where the first byte of the array written indicates the block length. Instead, the whole block is written to GDDRAM. However, ioctl() determines the writing length from that first byte while also writes it to the I2C interface. Therefore, ioctl() could not be used on SSD1306. We thus use write() directly.
int SSD1306BufferFlush(struct SSD1306_dispconf *disp){
	SSD1306Ctrl(disp,COLUMNADDR);
	SSD1306Ctrl(disp,0);
	SSD1306Ctrl(disp,disp->_width-1);
	SSD1306Ctrl(disp,PAGEADDR);
	SSD1306Ctrl(disp,0);
	SSD1306Ctrl(disp,disp->_pages-1);
	uint8_t data[disp->_bufferlen+1];
	memcpy(data+1,disp->buffer,disp->_bufferlen);
	data[0]=SSD1306_CTRL_DATA;
	return write(disp->_i2c_dev_fd,data,disp->_bufferlen+1);
}

int SSD1306Init(struct SSD1306_dispconf *disp){
	SSD1306Ctrl(disp,DISPLAYOFF);
	SSD1306Ctrl(disp,SETDISPLAYCLOCKDIV);
	SSD1306Ctrl(disp,0x80);
	SSD1306Ctrl(disp,SETMULTIPLEX);
	SSD1306Ctrl(disp,0x3F);
	SSD1306Ctrl(disp,SETDISPLAYOFFSET);
	SSD1306Ctrl(disp,0x0);
	SSD1306Ctrl(disp,SETSTARTLINE|0x0);
	SSD1306Ctrl(disp,CHARGEPUMP);
	if (disp->_vccstate==EXTERNALVCC) SSD1306Ctrl(disp,0x10);
	else SSD1306Ctrl(disp,0x14);
	SSD1306Ctrl(disp,MEMORYMODE);
	SSD1306Ctrl(disp,0x00);
	SSD1306Ctrl(disp,SEGREMAP|0x1);
	SSD1306Ctrl(disp,COMSCANDEC);
	SSD1306Ctrl(disp,SETCOMPINS);
	SSD1306Ctrl(disp,0x12);
	SSD1306Ctrl(disp,SETCONTRAST);
	if (disp->_vccstate==EXTERNALVCC) SSD1306Ctrl(disp,0x9F);
	else SSD1306Ctrl(disp,0xCF);
	SSD1306Ctrl(disp,SETPRECHARGE);
	if (disp->_vccstate==EXTERNALVCC) SSD1306Ctrl(disp,0x22);
	else SSD1306Ctrl(disp,0xF1);
	SSD1306Ctrl(disp,SETVCOMDETECT);
	SSD1306Ctrl(disp,0x40);
	SSD1306Ctrl(disp,DISPLAYRESUME);
	SSD1306Ctrl(disp,COLORNORMAL);
	SSD1306Ctrl(disp,DISPLAYON);
	return 0;
}

// Convert a width-by-height sized 1d-array of either uint8_t or char to a SSD1306 buffer. Consider positive only when pixel brightness larger than 127. Currently unused since layers are implemented in binary mode.
int SSD1306BufferFromArrayGreyscale(struct SSD1306_dispconf *disp,uint8_t *frame){
	memset(disp->buffer,0,disp->_bufferlen);
	for (int i=0;i<disp->_width;i++)
		for (int j=0;j<disp->_pages;j++)
			for (int p=0;p<8;p++)
				if (frame[i*disp->_height+j*8+p]>127) disp->buffer[j*disp->_width+i] += bitset[p];
	return 0;
}


// Convert a width-by-height sized 1d-array of either uint8_t or char to a SSD1306 buffer. Consider positive for all non-zero pixel.
int SSD1306BufferFromArrayBinary(struct SSD1306_dispconf *disp,uint8_t *frame){
	memset(disp->buffer,0,disp->_bufferlen);
	for (int i=0;i<disp->_width;i++)
		for (int j=0;j<disp->_pages;j++)
			for (int p=0;p<8;p++)
				if (frame[i*disp->_height+j*8+p]) disp->buffer[j*disp->_width+i] += bitset[p];
	return 0;
}

int SSD1306SetContrast(struct SSD1306_dispconf *disp,uint8_t contrast){
	SSD1306Ctrl(disp,SETCONTRAST);
	return SSD1306Ctrl(disp,contrast);
}

struct SSD1306_dispconf* SSD1306_128_64(int busnum,uint8_t addr){
	struct SSD1306_dispconf *disp = malloc(sizeof(struct SSD1306_dispconf));
	disp->_width = 128;
	disp->_height = 64;
	disp->_pages = 8;
	disp->_bufferlen = 128*8;
	disp->_displen = 128*64;
	disp->_vccstate = SWITCHCAPVCC;
	disp->_i2c_dev_fd = I2CInitByDevnum(busnum,addr);
	disp->contrast = 0xCF;
	disp->buffer = malloc(sizeof(uint8_t)*128*8);
	SSD1306Init(disp);
	return disp;
}

void SSD1306Destroy(struct SSD1306_dispconf *disp){
	I2CClose(disp->_i2c_dev_fd);
	free(disp->buffer);
	free(disp);
}
