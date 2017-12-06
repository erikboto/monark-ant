#!/usr/bin/python

import dbus
import gobject
import RPi.GPIO as GPIO
from dbus.mainloop.glib import DBusGMainLoop


def button_pressed(channel):
    print monarkControl.targetPower()

DBusGMainLoop(set_as_default=True)

dbusml = DBusGMainLoop()
session_bus = dbus.SessionBus()

monark = session_bus.get_object("se.unixshell", "/Monark")
monarkControl = dbus.Interface(monark, "se.unixshell.MonarkControl")


monarkControl.setTargetPower(200)
ml = gobject.MainLoop()

GPIO.setmode(GPIO.BCM)
GPIO.setup(26, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.add_event_detect(26, GPIO.FALLING, callback=button_pressed, bouncetime=100)

gobject.threads_init()
ml.run()
