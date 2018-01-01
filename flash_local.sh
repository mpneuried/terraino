#!/bin/sh

rm -f firmware.bin
particle compile core ./firmware --saveTo firmware.bin
particle flash --usb firmware.bin