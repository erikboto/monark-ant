#!/usr/bin/python

import dbus
from gi.repository import GObject
import RPi.GPIO as GPIO
from dbus.mainloop.glib import DBusGMainLoop


def left_button_pressed(channel):
    monarkControl.incGear()

def right_button_pressed(channel):
    monarkControl.decGear()

DBusGMainLoop(set_as_default=True)

dbusml = DBusGMainLoop()
session_bus = dbus.SessionBus()

monark = session_bus.get_object("se.unixshell", "/Monark")
monarkControl = dbus.Interface(monark, "se.unixshell.MonarkControl")


monarkControl.setTargetPower(200)
ml = GObject.MainLoop()

GPIO.setmode(GPIO.BCM)
GPIO.setup(26, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(4, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.add_event_detect(4, GPIO.FALLING, callback=left_button_pressed, bouncetime=100)
GPIO.add_event_detect(26, GPIO.FALLING, callback=right_button_pressed, bouncetime=100)

# GObject.timeout_add(1000, helloer)
GObject.threads_init()
ml.run()
