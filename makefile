all:
	gcc -c i2c.c -o i2c.o
	gcc -c SSD1306.c -o SSD1306.o
	gcc -o main i2c.o SSD1306.o