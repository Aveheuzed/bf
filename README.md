# Brainfuck machine for Arduino

This repo is an Arduino sketch implementing a Brainfuck machine for Arduino-compatible boards.

It is based on the code for the Ubuntu package `bf`.

Details on the package can be found at https://packages.ubuntu.com/jammy/bf.

This sketch depends on the SD Arduino library (https://www.arduino.cc/reference/en/libraries/sd/).
The code to run is expected to be at /prog.bf on the SD card.

Note: due to the limited RAM available on most Arduino boards, not all examples may work.
Notably, mandelbrot.b is known to be to big to fit in RAM, even on an Arduino Mega2560 rev3 (8Ki RAM).

Code in this repo is the property of their respective owners:
Stephan Beyer originally, then Alexis Masson (@Aveheuzed) for the edits he made.
License info can be found at the top of each file.
