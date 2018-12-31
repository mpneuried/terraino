#!/bin/sh

rm -f firmware.bin
particle compile core --target 0.6.0 ./firmware --saveTo firmware.bin
particle flash --usb firmware.bin