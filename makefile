all:
	gcc -c i2c.c -o i2c.o -Wall
	gcc -c SSD1306.c -o SSD1306.o -Wall
	gcc -I/usr/include/freetype2 -c gfxfunc.c -o gfxfunc.o -Wall
	gcc -I/usr/include/freetype2 -c display.c -o display.o -Wall
	gcc -c keyboard.c -o keyboard.o -Wall
	gcc -I/usr/include/freetype2 -c sys_frame.c -o sys_frame.o -Wall
	gcc -I/usr/include/freetype2 -c sys_menu.c -o sys_menu.o -Wall
	gcc -I/usr/include/freetype2 -c calcos.c -o calcos.o -Wall
	gcc -o calcos i2c.o SSD1306.o gfxfunc.o display.o keyboard.o sys_frame.o sys_menu.o calcos.o -li2c -lfreetype -lnetpbm -lgpiod -lpthread -Wall

debug:
	gcc -g -c i2c.c -o i2c.o -Wall -lefence
	gcc -g -c SSD1306.c -o SSD1306.o -Wall -lefence
	gcc -g -I/usr/include/freetype2 -c gfxfunc.c -o gfxfunc.o -Wall -lefence
	gcc -g -I/usr/include/freetype2 -c display.c -o display.o -Wall -lefence
	gcc -g -c keyboard.c -o keyboard.o -Wall -lefence
	gcc -g -I/usr/include/freetype2 -c sys_frame.c -o sys_frame.o -Wall -lefence
	gcc -g -I/usr/include/freetype2 -c sys_menu.c -o sys_menu.o -Wall -lefence
	gcc -g -I/usr/include/freetype2 -c calcos.c -o calcos.o -Wall -lefence
	gcc -g -o calcos i2c.o SSD1306.o gfxfunc.o display.o keyboard.o sys_frame.o sys_menu.o calcos.o -li2c -lfreetype -lnetpbm -lgpiod -lpthread -Wall -lefence
