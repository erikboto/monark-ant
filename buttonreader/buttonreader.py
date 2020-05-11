#!/usr/bin/python3 -u

import dbus
from gi.repository import GLib
from dbus.mainloop.glib import DBusGMainLoop
import serial
import threading

DBusGMainLoop(set_as_default=True)
session_bus = dbus.SessionBus()

monark = session_bus.get_object("se.unixshell", "/Monark")
monarkControl = dbus.Interface(monark, "se.unixshell.MonarkControl")


class KeyReader:
    def __init__(self, port, baud_rate):
        self.__port = port
        self.__baud_rate = baud_rate
        self.__thread = None
        self.__should_stop = False
        self._callbacks = []
        self.__serial_port = None

    def start(self):
        if not self.__thread:
            self.__thread = threading.Thread(target=self.__thread_function)
            self.__thread.start()

    def stop(self):
        self.__should_stop = True
        self.__thread.join()
        self.__thread = None

    def add_callback(self, cb):
        """
        Add a callback which is called when new data is available.

        Args:
            cb: The callable to add to the call list
        """
        if not cb in self._callbacks:
            self._callbacks.append(cb)

    def remove_callback(self, cb):
        """
        Remove a callback from the call list.

        Args:
            cb: The callable to remove from the call list
        """
        if cb in self._callbacks:
            self._callbacks.remove(cb)

    def __thread_function(self):
        self.__serial_port = serial.Serial(self.__port, self.__baud_rate, timeout=1)
        while not self.__should_stop:
            line = self.__serial_port.readline()
            if line:
                parts = line.decode('utf-8').strip().split(' ')
                if len(parts) == 2:
                    for cb in self._callbacks:
                        cb(parts[0], parts[1])


def handle_key(key, type):
    print("key: {}, type: {}".format(key, type))
    if key == 'right' and type == 'short':
        monarkControl.incGear()
    elif key == 'right' and type == 'long':
        monarkControl.incGearLots()
    elif key == 'left' and type == 'short':
        monarkControl.decGear()
    elif key == 'left' and type == 'long':
        monarkControl.decGearLots()


kr = KeyReader(port="/dev/ttyACM0", baud_rate=115200)
kr.start()
kr.add_callback(handle_key)
monarkControl.setTargetPower(200)
ml = GLib.MainLoop()

# GLib.timeout_add(1000, helloer)
ml.run()
