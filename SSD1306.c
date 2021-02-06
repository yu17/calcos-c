#include "SSD1306.h"

int SSD1306_ctrl(struct SSD1306_dispconf *disp,uint8_t command){
	int ret = I2CWriteRegByte(disp->_i2c_dev_fd,SSD1306_CTRL_COMM,command);
	if (ret<0) {
		printf("Failed to communicate command 0X%X with errno %d.\n",command,errno);
		exit(1);
	}
	return ret;
}

int SSD1306_data_byte(struct SSD1306_dispconf *disp,uint8_t data){
	return I2CWriteRegByte(disp->_i2c_dev_fd,SSD1306_CTRL_DATA,data);
}

int SSD1306_init(struct SSD1306_dispconf *disp){
	SSD1306_ctrl(disp,DISPLAYOFF);
	SSD1306_ctrl(disp,SETDISPLAYCLOCKDIV);
	SSD1306_ctrl(disp,0x80);
	SSD1306_ctrl(disp,SETMULTIPLEX);
	SSD1306_ctrl(disp,0x3F);
	SSD1306_ctrl(disp,SETDISPLAYOFFSET);
	SSD1306_ctrl(disp,0x0);
	SSD1306_ctrl(disp,SETSTARTLINE|0x0);
	SSD1306_ctrl(disp,CHARGEPUMP);
	if (disp->_vccstate==EXTERNALVCC) SSD1306_ctrl(disp,0x10);
	else SSD1306_ctrl(disp,0x14);
	SSD1306_ctrl(disp,MEMORYMODE);
	SSD1306_ctrl(disp,0x00);
	SSD1306_ctrl(disp,SEGREMAP|0x1);
	SSD1306_ctrl(disp,COMSCANDEC);
	SSD1306_ctrl(disp,SETCOMPINS);
	SSD1306_ctrl(disp,0x12);
	SSD1306_ctrl(disp,SETCONTRAST);
	if (disp->_vccstate==EXTERNALVCC) SSD1306_ctrl(disp,0x9F);
	else SSD1306_ctrl(disp,0xCF);
	SSD1306_ctrl(disp,SETPRECHARGE);
	if (disp->_vccstate==EXTERNALVCC) SSD1306_ctrl(disp,0x22);
	else SSD1306_ctrl(disp,0xF1);
	SSD1306_ctrl(disp,SETVCOMDETECT);
	SSD1306_ctrl(disp,0x40);
	SSD1306_ctrl(disp,DISPLAYRESUME);
	SSD1306_ctrl(disp,COLORNORMAL);
	SSD1306_ctrl(disp,DISPLAYON);
	return 0;
}

// SSD1306 does not adopt standard block data writing, where the first byte of the array written indicates the block length. Instead, the whole block is written to GDDRAM. However, ioctl() determines the writing length from that first byte while also writes it to the I2C interface. Therefore, ioctl() could not be used on SSD1306. We thus use write() directly.
int SSD1306_buffer_flush(struct SSD1306_dispconf *disp){
	SSD1306_ctrl(disp,COLUMNADDR);
	SSD1306_ctrl(disp,0);
	SSD1306_ctrl(disp,disp->_width-1);
	SSD1306_ctrl(disp,PAGEADDR);
	SSD1306_ctrl(disp,0);
	SSD1306_ctrl(disp,disp->_pages-1);
	uint8_t data[disp->_bufferlen+1];
	memcpy(data+1,disp->buffer,disp->_bufferlen);
	data[0]=SSD1306_CTRL_DATA;
	return write(disp->_i2c_dev_fd,data,disp->_bufferlen+1);
}

// Convert a width-by-height sized 1d-array of either uint8_t or char to a SSD1306 buffer.
int SSD1306_buffer_from_1darray(struct SSD1306_dispconf *disp,uint8_t *canvas){
	memset(disp->buffer,0,disp->_bufferlen);
	for(int i=0;i<disp->_width;i++)
		for(int j=0;j<disp->_pages;j++)
			for(int p=0;p<8;p++)
				if (canvas[i*disp->_height+j*8+p]>127) disp->buffer[j*disp->_width+i] += bitset[p];
	return 0;
}

struct SSD1306_dispconf* SSD1306_128_64(int busnum,uint8_t addr){
	struct SSD1306_dispconf *disp = malloc(sizeof(struct SSD1306_dispconf));
	disp->_width = 128;
	disp->_height = 64;
	disp->_pages = 8;
	disp->_bufferlen = 128*8;
	disp->_canvaslen = 128*64;
	disp->_vccstate = SWITCHCAPVCC;
	disp->_i2c_dev_fd = I2CInitByDevnum(busnum,addr);
	disp->contrast = 0xCF;
	disp->buffer = malloc(sizeof(uint8_t)*128*8);
	SSD1306_init(disp);
	return disp;
}

void SSD1306_destroy(struct SSD1306_dispconf *disp){
	free(disp->buffer);
	free(disp);
}

int main(){
	struct SSD1306_dispconf* disp = SSD1306_128_64(0,0x3C);
	uint8_t testarr[128*64];
	memset(testarr,0,sizeof(testarr));
	testarr[0]=255;
	testarr[128*64-1]=255;
	SSD1306_buffer_from_1darray(disp,testarr);
	SSD1306_buffer_flush(disp);
	return 0;
}
