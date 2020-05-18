#!/usr/bin/python3

import serial
import time
import random

s = serial.Serial('/tmp/monark-test2', 4800, timeout=0)



while True:
    foo = s.read(100).decode('utf-8')
    foo = foo.replace("\r","")
    if len(foo) > 0:
        print("Got cmd:", foo)
        print(repr(foo))

    if foo.startswith("id"):
        s.write("novofake\r".encode('utf-8'))
    if foo.startswith("pedal"):
        val = int(90 + random.randint(-5,5))
        s.write(("%s\r" % val).encode('utf-8'))
        print ("Sent:", val)
    if foo.startswith("pulse"):
        val = int(150 + random.randint(-5,5))
        s.write(("%s\r" % val).encode('utf-8'))
        print("Sent:", val)
    if foo.startswith("power"):
        val = int(250 + random.randint(-10,10))
        s.write(("%s\r" % val).encode('utf-8'))
        print("Sent:", val)
    time.sleep(0.01)
    #if foo.beginswith("id"):
