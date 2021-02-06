#include "i2c.h"

int I2CInit(const char *path,const int i2c_dev_addr){
	int i2c_dev_fd;
	i2c_dev_fd = open(path,O_RDWR);
	if (i2c_dev_fd<0) {
		printf("Failed to open I2C device %s with error %d!\n",path,errno);
		exit(1);
	}
	if (ioctl(i2c_dev_fd,I2C_SLAVE,i2c_dev_addr)<0) {
		printf("Failed to initialize connection with I2C slave device on bus %s at address 0X%X with errno %d!\n",path,i2c_dev_addr,errno);
		exit(1);
	}
	return i2c_dev_fd;
}

int I2CInitByDevnum(const int i2c_dev_num,const int i2c_dev_addr){
	char devfname[20];
	if (i2c_dev_num>=0 && i2c_dev_num<=6) {
		snprintf(devfname,19,"/dev/i2c-%d",i2c_dev_num);
		return I2CInit(devfname,i2c_dev_addr);
	}
	else {
		printf("Invalid I2C device number!\n");
		exit(1);
	}
}

// Most of the following code were borrowed from linux/i2c or linux/i2c-dev, allowing user space usage of i2c functions.

int I2CReadByte(uint8_t i2c_dev_fd){
	union i2c_smbus_data data;
	if (i2c_smbus_access(i2c_dev_fd,I2C_SMBUS_READ,0,I2C_SMBUS_BYTE,&data)) return -1;
	else return data.byte & 0xFF;
}

int I2CReadRegByte(uint8_t i2c_dev_fd,uint8_t i2c_dev_reg){
	union i2c_smbus_data data;
	if (i2c_smbus_access(i2c_dev_fd,I2C_SMBUS_READ,i2c_dev_reg,I2C_SMBUS_BYTE_DATA,&data)) return -1;
	else return data.byte & 0xFF;
}

int I2CReadRegWord(uint8_t i2c_dev_fd,uint8_t i2c_dev_reg){
	union i2c_smbus_data data;
	if (i2c_smbus_access(i2c_dev_fd,I2C_SMBUS_READ,i2c_dev_reg,I2C_SMBUS_WORD_DATA,&data)) return -1;
	else return data.word & 0xFFFF;
}

int I2CWriteByte(uint8_t i2c_dev_fd,uint8_t value){
	return i2c_smbus_access(i2c_dev_fd,I2C_SMBUS_WRITE,value,I2C_SMBUS_BYTE,NULL);
}

int I2CWriteRegByte(uint8_t i2c_dev_fd,uint8_t reg,uint8_t value){
	union i2c_smbus_data data;
	data.byte = value;
	return i2c_smbus_access(i2c_dev_fd,I2C_SMBUS_WRITE,reg,I2C_SMBUS_BYTE_DATA,&data);
}

int I2CWriteRegWord(uint8_t i2c_dev_fd,uint8_t reg,uint8_t value){
	union i2c_smbus_data data;
	data.word = value;
	return i2c_smbus_access(i2c_dev_fd,I2C_SMBUS_WRITE,reg,I2C_SMBUS_WORD_DATA,&data);
}

int I2CProcessCall(uint8_t i2c_dev_fd,uint8_t reg,uint8_t value)
{
	union i2c_smbus_data data;
	data.word = value;
	if (i2c_smbus_access(i2c_dev_fd,I2C_SMBUS_WRITE,reg,I2C_SMBUS_PROC_CALL,&data)) return -1;
	else return 0x0FFFF & data.word;
}

/* Returns the number of read bytes */
int I2CReadRegBlock(uint8_t i2c_dev_fd,uint8_t reg,uint8_t *values){
	union i2c_smbus_data data;
	if (i2c_smbus_access(i2c_dev_fd,I2C_SMBUS_READ,reg,I2C_SMBUS_BLOCK_DATA,&data)) return -1;
	else {
		for (int i=1;i<=data.block[0];i++)
			values[i-1] = data.block[i];
		return data.block[0];
	}
}

int I2CWriteRegBlock(uint8_t i2c_dev_fd,uint8_t reg,uint8_t length,uint8_t *values){
	union i2c_smbus_data data;
	if (length>32) length = 32;
	for (int i=1;i<=length;i++) data.block[i] = values[i-1];
	data.block[0] = length;
	return i2c_smbus_access(i2c_dev_fd,I2C_SMBUS_WRITE,reg,I2C_SMBUS_BLOCK_DATA,&data);
}