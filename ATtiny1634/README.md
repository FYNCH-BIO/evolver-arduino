avr-gcc -g -Os -mmcu=attiny1634 -c lightsense.c
avr-gcc -g -mmcu=attiny1634 -o lightsense.elf lightsense.o
avr-objcopy -j .text -j .data -O ihex lightsense.elf lightsense.hex
avrdude -p t1634 -P /dev/cu.usbmodem14611 -c arduino -b 19200 -U flash:w:lightsense.hex -v -v -v -v
