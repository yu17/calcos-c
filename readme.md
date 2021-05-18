# Raspberry Pi Zero W Based Hybrid Calculator Operating System

This software is written as the operating system for the hand-made calculator. As the calculator involves a hand-welded keyboard and hand-made display made of three SSD1306 128\*64 mono-color white OLED displays, appropriate drivers are developed so as to have these hardware working properly.

Some details about the implementation are noted down below for future reference.

## I2C Interface

The displays use I2C interface for communication. However, the well-known Wiring-Pi library has stopped maintenance and thus deprecated, leaving us no easy way for handling I2C communication. Therefore, most of the I2C operations are done via `i2c/smbus.h`, `linux/i2c.h`, and `linux/i2c-dev.h` libraries. These header files contain necessary information on how to use those APIs for I2C communication.

## Display

The display consists of three SSD1306 mono-color white OLED displays. Each has the resolution 128\*64, offering a total of 384\*64 pixels. Gaps exists at x=128 and 256, so the displays are usually used as three separate ones instead of as a whole.

Three files are involved for the display functionalities.

### SSD1306 Device Driver

`SSD1306.c` implements the driver for the physical devices.

The way data blocks are transferred to SSD1306 via I2C is abnormal. Conventionally, the first byte of the block should be the length of the data block. However, in the case of the SSD1306, no length information is sent and the data starts from the first byte. Therefore, a dedicated data block transfer function is implemented.

The driver handles the initialization and shutdown of the physical display, and flushes the buffer to the display. It offers the buffer level interface to be used in the `display.c`. For each physical device, a `struct` called `SSD1306_dispconf` is created so as to store the configuration.

### Screen Management

`display.c` implements the actual infrastructure for the operating system and the applications to use. Three physical devices are combined here with the structure called `screen`.

A `screen` could have multiple `layers`, each of which are assigned to an application to be used, and each are to be rendered on to the `frame`, which is then flushed to the physical devices. [Missing details]

A `layer` could then have multiple `gfxobjects`.

### Display Starting Routing

## Keyboard

### Keyboard Scanner

### Input Method

## OS Framework

### Boot and Shutdown Routines

### Application Routines

To start an application:

1. Construct the graphics layer for the application.

2. Assign the layer to the corresponding `apppacket`.

3. Add the layer to the `screen`.

4. Mark app as running and foreground in the `apppacket`.

5. Call the init function of the app. The rest of the `apppacket` should be initialized here.

6. Set the `CharHandler` as the one in the `apppacket`.

7. Push the `apppacket` to the top of the `running_apps` stack.

To terminate an application:

1. Call the term function of the app. All the active threads of the app should be canceled/joined here.

2. Mark app as not-running and not-foreground in the `apppacket`.

3. Clear the `CharHandler`.

4. Remove the layer from the `screen` (and implicitly destroy it).

5. Pop the `apppacket` from the `running_apps` stack.

6. Clear `apppacket` structure.