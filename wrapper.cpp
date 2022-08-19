/*
Wrapper around the Arduino I/O functions for use in bf.c
The program is loaded from an SD card (as supported by Arduino's SD library).
In the current implementation, the Chip Select pin is pin 4 (e.g., for Arduino's Ethernet shield).
Change the value of SDPIN to fit your needs.
The file loaded is "/prog.bf". Again, change FILENAME as needed.

All user I/O happens on Serial, with the standard baud rate of 9600.

(C) GPL Alexis Masson, 2022
*/


#include "stdlib.h"

#include "wrapper.h"

#include <Arduino.h>
#include <SD.h>

#include "errors.h"

#define SDPIN 4

#define FILENAME "prog.bf"

extern "C" {

void ttyInit(void) {
	Serial.begin(9600);
  while (!Serial);
}
char ttyGetInput(void) {
	return Serial.read();
}


static File staging_file;
void SDopen() {
	if (!SD.begin(SDPIN)) {
		ErrorMsg("Error: Couldn't initialize the SD card");
  }
	staging_file = SD.open(FILENAME, FILE_READ);
	if (!staging_file) {
		ErrorMsg("Error: Couln't find /" FILENAME " on the SD card:");
  }
}
void SDclose(void) {
	staging_file.close();
}
size_t SDread(void *ptr, size_t size, size_t nmemb) {
	return staging_file.read(ptr, size*nmemb);
}
int SDeof(void) {
	return !staging_file.available();
}

int putchar(int c) {
	Serial.write(c);
  Serial.flush();
	return c;
}

int puts(const char* s) {
	int len = Serial.println(s);
	Serial.flush();
	return len;
}

}
