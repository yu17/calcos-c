#ifndef I2C_usutil
#define I2C_usutil 0.1

#include <stdio.h>
#include <stdlib.h>
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

static inline int i2c_smbus_access(int file,char read_write,uint8_t command,int size,union i2c_smbus_data *data){
	struct i2c_smbus_ioctl_data args;
	args.read_write = read_write;
	args.command = command;
	args.size = size;
	args.data = data;
	return ioctl(file,I2C_SMBUS,&args);
}

int I2CInit(const char *path,const int i2c_dev_addr);

int I2CInitByDevnum(const int i2c_dev_num,const int i2c_dev_addr);

int I2CReadByte(uint8_t i2c_dev_fd);

int I2CReadRegByte(uint8_t i2c_dev_fd,uint8_t i2c_dev_reg);

int I2CReadRegWord(uint8_t i2c_dev_fd,uint8_t i2c_dev_reg);

int I2CWriteByte(uint8_t i2c_dev_fd,uint8_t value);

int I2CWriteRegByte(uint8_t i2c_dev_fd,uint8_t reg,uint8_t value);

int I2CWriteRegWord(uint8_t i2c_dev_fd,uint8_t reg,uint8_t value);

int I2CProcessCall(uint8_t i2c_dev_fd,uint8_t reg,uint8_t value);

/* Returns the number of read bytes */
int I2CReadRegBlock(uint8_t i2c_dev_fd,uint8_t reg,uint8_t *values);

int I2CWriteRegBlock(uint8_t i2c_dev_fd,uint8_t reg,uint8_t length,uint8_t *values);

#endif