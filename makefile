all:
	gcc -c i2c.c -o i2c.o -Wall
	gcc -c SSD1306.c -o SSD1306.o -Wall
	gcc -I/usr/include/freetype2 -c gfxfunc.c -o gfxfunc.o -Wall
	gcc -I/usr/include/freetype2 -c display.c -o display.o -Wall
	gcc -c keyboard.c -o keyboard.o -Wall
	gcc -I/usr/include/freetype2 -c calcos.c -o calcos.o -Wall
	gcc -o calcos i2c.o SSD1306.o gfxfunc.o display.o keyboard.o calcos.o -li2c -lfreetype -lnetpbm -lgpiod -lpthread -Wall
