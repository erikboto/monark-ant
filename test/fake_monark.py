#!/usr/bin/python

import serial
import time
import random

s = serial.Serial('/tmp/monark-test2', 4800, timeout=0)

while True:
    foo = str(s.read(100))
    foo = foo.replace("\r","")
    if len(foo) > 0:
        print "Got cmd:", foo

    if foo.startswith("id"):
        s.write("novofake\r")
    if foo.startswith("pedal"):
        val = int(90 + random.randint(-5,5))
        s.write("%s\r" % val)
        print "Sent:", val
    if foo.startswith("pulse"):
        val = int(150 + random.randint(-5,5))
        s.write("%s\r" % val)
        print "Sent:", val
    if foo.startswith("power"):
        val = int(250 + random.randint(-10,10))
        s.write("%s\r" % val)
        print "Sent:", val
    time.sleep(0.01)
    #if foo.beginswith("id"):
