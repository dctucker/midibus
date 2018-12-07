#!/usr/bin/env python2
# -*- coding: utf-8 -*-

import sys
import rtmidi
import signal
import locale
locale.setlocale(locale.LC_ALL, '')
import curses

#from matrix import View

class View:
	def __init__(self, screen, model):
		self.screen = screen
		self.cursor = [0,0]
		self.model = model

	def input_left(self):
		return 1 + len(self.model.inputs) * 2

	def output_top(self):
		return 1 + len(self.model.inputs)

	def draw(self):
		model = self.model
		s = self.screen
		y = 0
		x = self.input_left() - 1
		for i in model.inputs:
			s.move(y, x)
			s.addch(curses.ACS_HLINE)
			s.addstr(y, x + 1, i)
			y += 1
		max_y = y
		y += len(model.outputs)
		#y = 1
		x = 0
		for o in model.outputs:
			s.addstr(y, x, o)
			y -= 1
			x += 2
		
		y = self.output_top() - 1
		for x in range(len(model.outputs)):
			s.move(y, x * 2)
			s.vline(curses.ACS_VLINE, len(model.outputs) - x)

		for y in range(len(model.inputs)):
			for x in range(len(model.outputs)):
				s.move(y, x * 2)
				s.addch(curses.ACS_PLUS)
				s.addch(curses.ACS_HLINE)

		for p1, p2 in model.connections:
			y = model.get_index_in(p1)
			x = model.get_index_out(p2) * 2
			s.move(y,x)
			s.addch(curses.ACS_DIAMOND)
			s.addstr(y, x, "◊")
		s.move( self.output_top() + len(model.outputs) + 2, 0)

	def set_cursor(self, cursor):
		s = self.screen
		s.chgat( cursor[0], self.input_left(), len( self.model.inputs[cursor[0]] ), curses.A_REVERSE )
		s.chgat( len( self.model.outputs ) - cursor[1] + self.output_top() - 1, 2*cursor[1], len( self.model.outputs[cursor[1]] ), curses.A_REVERSE )
		self.cursor = cursor
		self.screen.move( cursor[0], cursor[1] * 2 )


class Devices:
	_instance = None
	def __init__(self):
		self.open_inputs = {}
		self.open_outputs = {}
		self.connections = []
		self.load()

	@classmethod
	def instance(cls):
		if cls._instance is None:
			cls._instance = cls()
		return cls._instance

	def load(self):
		self.ins()
		self.outs()

	def ins(self):
		self.inputs = {}
		rt = rtmidi.RtMidiIn()
		for i in range(rt.getPortCount()):
			name = ' '.join(rt.getPortName(i).split(' ')[:-1])
			self.inputs[ name ] = i

	def input(self,name):
		if name not in self.inputs:
			raise Exception("Device '%s' not found" % name)
		if name not in self.open_inputs:
			rt = rtmidi.RtMidiIn()
			rt.openPort( self.inputs[name] )
			#rt.ignoreTypes(False, False, False)
			self.open_inputs[name] = rt
		return self.open_inputs[name]

	def outs(self):
		self.outputs = {}
		rt = rtmidi.RtMidiOut()
		for i in range(rt.getPortCount()):
			name = ' '.join(rt.getPortName(i).split(' ')[:-1])
			self.outputs[ name ] = i

	def output(self, name):
		if name not in self.outputs:
			raise Exception("Device '%s' not found" % name)
		if name not in self.open_outputs:
			rt = rtmidi.RtMidiOut()
			rt.openPort( self.outputs[name] )
			self.open_outputs[name] = rt
		return self.open_outputs[name]

	def __del__(self):
		for rt in self.open_outputs.values():
			rt.closePort()
		for rt in self.open_inputs.values():
			rt.closePort()

class DeviceListener:
	channels = [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16]
	def __init__(self, name):
		self.name = name
		self.input = Devices.instance().input(self.name)
		self.input.setCallback( self )
		self.output = Devices.instance().output(self.name)
		self.subs = []

	def __call__(self, message):
		data = message.getRawData()
		print '\033[2K', 
		print ' '.join([hex(ord(d))[2:] for d in data]),
		self.forward(message)
		print "\r",
		sys.stdout.flush()

	def forward(self, message):
		for sub in self.subs:
			if sub.filter(message):
				sub.output.send(message)

	def send(self, message):
		data = message.getRawData()
		print self.name,
		self.output.sendMessage(message)

	def send_unison(self, message, channel):
		msg = rtmidi.MidiMessage(message)
		msg.setChannel(channel)
		self.output.sendMessage(msg)

	def subscribe(self, other):
		if other not in self.subs:
			self.subs.append(other)

class Connection:
	def __init__(self, i, o):
		self.input = i
		self.output = o
		self.filter = lambda x: True
		
	def connect(self):
		devs = Devices.instance()
		self.input.subscribe(self)

	def __eq__(self, other):
		return self.input.name == other.input.name and self.output.name == other.output.name

class Router:
	def __init__(self):
		self.connections = []
		self.listeners = {}

	def connect(self, i, o, filt=lambda x: True):
		if i not in self.listeners:
			self.listeners[i] = DeviceListener(i)
		if o not in self.listeners:
			self.listeners[o] = DeviceListener(o)

		i = self.listeners[i]
		o = self.listeners[o]

		new_connection = Connection(i,o)
		new_connection.filter = filt
		new_connection.connect()
		for connection in self.connections:
			if connection == new_connection:
				self.connections.remove(connection)
		self.connections.append(new_connection)

def main(stdscr):
	def channel_filter(channels):
		return lambda x: x.getChannel() in channels

	devices = Devices.instance()
	devices.load()
	router = Router()
	router.connect("Deluge", "Little Phatty SE II", channel_filter((3,)))
	router.connect("Deluge", "Circuit", channel_filter((0,1,2,10,)))
	router.connect("Deluge", "JUNO-DS", channel_filter((4,5,6,7,9,11,)))
	router.connect("JUNO-DS", "Circuit", channel_filter((1,2,10,)))
	router.connect("JUNO-DS", "Deluge", channel_filter((8,)))
	router.connect("JUNO-DS", "Little Phatty SE II", channel_filter((3,)))
	router.connect("BCR2000", "Little Phatty SE II", channel_filter((3,)))

	def hup(signum, frame):
		devices.load()
	signal.signal(signal.SIGHUP, hup)

	#while True: #debug
	#	message = device.getMessage(100)
	#	call(message)

	#ctrl = Controller(devices)
	view = View(stdscr, devices)

	while True:
		view.draw()
		#view.set_cursor(ctrl.cursor)
		key = stdscr.getch()
		if key == ord('q'):
			break
		elif key == ord('r'):
			stdscr.clear()
			
		#ctrl.key_press(key)

if __name__ == '__main__':
	curses.wrapper(main)
