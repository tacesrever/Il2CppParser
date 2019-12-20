#!/usr/bin/python
import sys, os

os.system("wslbridge sh -c 'cd build;make'")
os.system("adb push build/libparser.so /data/local/tmp/libparser.so")
os.system("adb shell chmod 0755 /data/local/tmp/libparser.so")