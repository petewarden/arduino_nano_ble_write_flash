# Arduino Nano BLE Write Flash
An example of modifying flash memory on the Arduino Nano BLE Sense 33 from a sketch, using Mbed.

## Overview

I couldn't find complete examples that showed me how to initialize, modify, and write flash memory on the Arduino Nano BLE boards, so I've put this together to capture what I came up with, and hopefully help other people who need to do this. The main difference between this and other examples I've found is that I've persuaded the compiler to initialize the area of memory once during upload.
The example itself simply increments a counter variable once every time the board is reset. It reserves an area of 64KB to hold the data it's modifying, which is overkill for this application, but is what I need for some other use cases.

## Trying it out

To give this a try yourself:

 - Attach an Arduino Nano BLE Sense 33 board to your development machine.
 - Load the arduino_nano_ble_write_flash.ino sketch into the Arduino IDE.
 - Open the Serial Monitor.
 - Build and upload the sketch.

The first time the sketch runs, you should see `Counter=0` in the Serial Monitor. If you then reset the board with a single short press of its button, you should see `Counter=1`, with the number increasing by one every time its reset.

## How it works

There are comments inline in the code which should help explain some of how this is implemented. The most novel part is the use of a const byte array to reserve the memory we need in flash, using alignment and initialization tricks to make sure it's zero on first upload, and accessible by the [Mbed flash API](https://os.mbed.com/docs/mbed-os/v6.9/apis/flashiapblockdevice.html).

## Warnings

Accessing flash directly is an advanced topic, since it's possible to brick your board if you write to the wrong locations. It's also not advised to write repeatedly to flash (for example in your loop() function) since the memory only has a limited number of erase and program cycles it can support, and writing to flash can be slow and blocks a lot of other interrupts needed by the system.

## Related work

I'm grateful to the authors of the other examples I used to learn about flash access on modern Arduinos, including:

 - https://www.arduino.cc/pro/tutorials/portenta-h7/por-ard-kvs
 - https://os.mbed.com/docs/mbed-os/v6.9/apis/flashiapblockdevice.html
 - https://github.com/arduino/ArduinoCore-nRF528x-mbedos/issues/16#issuecomment-534877285

## Licensing

Available under the Apache 2 License, see LICENSE file for more details.

Pete Warden, pete@petewarden.com