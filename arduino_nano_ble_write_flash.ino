/* Copyright 2021 The TensorFlow Authors. All Rights Reserved.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

// This sketch demonstrates how to initialize, read, and update flash memory
// on an Arduino Nano BLE Sense 33. It contains a counter that starts at zero
// when the sketch is first uploaded, and is incremented by one every time that
// the board is reset without a re-flash. You can test this by uploading the
// sketch, opening the serial monitor, and seeing "Counter=0" output initially.
// If you then press the reset button, you should see "Counter=1", and so on as
// the board is restarted.

// This Mbed API lets us access flash memory, so we include the header. See
// https://os.mbed.com/docs/mbed-os/v6.9/apis/flashiapblockdevice.html
#include "FlashIAPBlockDevice.h"

// One of the challenges I had with the existing examples I found of using flash
// was that they didn't demonstrate how to set an initial value for the memory
// when the sketch was first uploaded. This was essential for most use cases,
// for example we want the counter to start at zero so the initial contents need
// to be set when the program is flashed. I would typically try to use linker
// scripts to set aside an area of memory and mark it so it's initialized during
// flashing, but that's not very easy with the Arduino IDE. Instead I use a bit of
// a hack, by:
//  - Declaring an array of bytes as const, so I know it will end up in
//    flash.
//  - Forcing it to align with the block size of the flash, so it can be
//    accessed by the API that works with whole pages of flash.
//  - Adding an empty initializer list ({}) tells the compiler to zero out the
//    contents in the binary that's flashed to the device.
// The last point is a bit subtle, because you might think that doing it in a
// constructor or setup() would work, but that would be called every time the
// program is reset. What I needed was a way to create a binary with that area
// already initialized before upload, which this approach offers.
// Some other examples use hard-coded addresses for the flash, usually towards
// the top of the address space so that they won't clash with the loaded program,
// but this approach does have the advantage that your writable area is dealt with
// in the same way as other variables, so you shouldn't have to worry about
// overwriting anything.

// From experimentation I know that the "page size" or "block size" of the flash
// on the board is 4KB, and should match the reported get_erase_size() from the
// block device object. In theory we should be able to use the
// MBED_CONF_FLASHIAP_BLOCK_DEVICE_SIZE macro, but this seems to be set to zero.
constexpr int kFlashBlockSize = 4096;

// Your flash area needs to be a multiple of the block size. In this case I want
// 64KB (even though I'm only using the first four bytes). You should make this
// the smallest multiple of the block size that will hold your data.
#define ROUND_UP(val, block_size) ((((val) + ((block_size) - 1)) / (block_size)) * (block_size))
constexpr int kFlashBufferSize = ROUND_UP(64 * 1024, kFlashBlockSize);

// As mentioned above, alignas ensures we get a valid block-aligned address,
// const should make sure it's in flash, and the empty brackets mean the array
// is filled with zeroes when the program is first uploaded.
alignas(kFlashBlockSize) const uint8_t flash_buffer[kFlashBufferSize] = {};

void setup() {
  Serial.begin(9600);

  // Beware! If you copy and paste this code into your own example, and try
  // running it without the serial monitor attached, this line will hang forever.
  // You'll need to remove this in production (and yes I have been caught by this
  // myself far too often).
  while (!Serial);

  // The address of the array will be determined by the compiler, and we need to
  // grab it as a 32-bit integer to use with the Mbed API.
  const uint32_t flash_buffer_address = reinterpret_cast<uint32_t>(flash_buffer);
  Serial.println(String("flash_buffer_address=0x") + String(flash_buffer_address, 16));

  // Create the flash object we'll be using to access the flash memory.
  static FlashIAPBlockDevice bd(flash_buffer_address, kFlashBufferSize);
  bd.init();

  // We'll need to copy the area of flash to RAM to modify it, so allocate an
  // array of the right size.  
  uint8_t* ram_buffer = (uint8_t*)(malloc(kFlashBufferSize));

  // Copy the existing flash contents into our ram buffer.  
  bd.read(ram_buffer, 0, kFlashBufferSize);

  // We need to erase the flash area before we can write into it again.
  bd.erase(0, kFlashBufferSize);

  // Treat the first four bytes of the copy of our flash array as a 32-bit
  // integer.
  int32_t* counter_address = reinterpret_cast<int32_t*>(ram_buffer);

  // Print out the value before any changes.
  Serial.println(String("Counter=") + String(*counter_address, 16));

  // Update our RAM copy of the counter integer.
  *counter_address += 1;

  // Write the whole copy of our RAM buffer back into flash. 
  bd.program(ram_buffer, 0, kFlashBufferSize);

  // Deallocate RAM buffer
  free(ram_buffer);

  // Shut down our flash access object.
  bd.deinit();  
}

void loop() {
  // Put your main code here, to run repeatedly.
  // Note that writing to flash can take some time, may block interrupts
  // needed for other parts of the system (like Bluetooth), and can wear out
  // the hardware if it's done too frequently. You probably don't want to
  // erase and program a block frequently, every time a loop() is called for
  // example.
}
