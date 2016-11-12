#!/bin/bash
esptool.py --port /dev/tty.SLAB_USBtoUART write_flash 0x0 .pioenvs/esp12e/firmware.bin

