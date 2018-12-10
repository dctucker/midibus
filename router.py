#!/usr/bin/env python2
# -*- coding: utf-8 -*-

import sys
import signal
import locale
locale.setlocale(locale.LC_ALL, '')
import curses

import devices

#from matrix import View

class Controller:
	def __init__(self, model):
		self.model = model
		self.cursor = [0,0]

	def reload(self):
		self.model.load()
		self.cursor = [0,0]

	def move_cursor(self, direction):
		new_i = self.cursor[0] + direction[0]
		if 0 <= new_i and new_i < len(self.model.inputs):
			self.cursor[0] = new_i
		new_j = self.cursor[1] + direction[1]
		if 0 <= new_j and new_j < len(self.model.outputs):
			self.cursor[1] = new_j

	def key_press(self, c):
		if c in (ord('h'), curses.KEY_LEFT):
			self.move_cursor((0, -1))
		elif c in (ord('l'), curses.KEY_RIGHT):
			self.move_cursor((0, 1))
		elif c in (ord('k'), curses.KEY_UP):
			self.move_cursor((-1, 0))
		elif c in (ord('j'), curses.KEY_DOWN):
			self.move_cursor((1, 0))
		elif c in (ord('r'),):
			self.reload()
		elif c in (ord("\n"),):
			i = self.model.inputs[ self.cursor[0] ]
			o = self.model.outputs[ self.cursor[1] ]
			self.model.connect(i,o)
			self.model.load()
		elif c in (ord("\b"),curses.KEY_BACKSPACE):
			i = self.model.inputs[ self.cursor[0] ]
			o = self.model.outputs[ self.cursor[1] ]
			self.model.disconnect(i,o)
			self.model.load()

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
			attr = curses.A_DIM if model.inputs[i] < 0 else curses.A_NORMAL
			s.addstr(y, x + 1, i, attr)
			y += 1
		max_y = y
		y += len(model.outputs)
		#y = 1
		x = 0
		for o in model.outputs:
			attr = curses.A_DIM if model.outputs[o] < 0 else curses.A_NORMAL
			s.addstr(y, x, o, attr)
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

		s.move(self.output_top() + len(model.outputs) + 1, 0)
		s.clrtoeol();
		for c in model.connections:
			y = model.inputs.keys().index(c.input.name)
			x = model.outputs.keys().index(c.output.name) * 2
			s.move(y,x)
			s.addch(curses.ACS_DIAMOND)
			s.addstr(y, x, "◊")
			if self.cursor == [y,x/2]:
				s.addstr(self.output_top() + len(model.outputs) + 1, 0, str(c.filter))

		s.move( self.output_top() + len(model.outputs) + 2, 0)

	def set_cursor(self, cursor):
		s = self.screen
		s.chgat( cursor[0], self.input_left(), len( self.model.inputs.keys()[cursor[0]] ), curses.A_REVERSE )
		s.chgat( len( self.model.outputs ) - cursor[1] + self.output_top() - 1, 2*cursor[1], len( self.model.outputs.keys()[cursor[1]] ), curses.A_REVERSE )
		self.cursor = cursor
		self.screen.move( cursor[0], cursor[1] * 2 )


class DeviceListener:
	channels = [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16]
	def __init__(self, name):
		self.name = name
		self.subs = []
		self.callback = False
		self.setup()

	def setup(self):
		if self.name in devices.inputs.keys() and self.name in devices.outputs.keys():
			self.input = devices.input(self.name)
			if not self.callback:
				self.input.setCallback( self )
				self.callback = True
			self.output = devices.output(self.name)
			self.listening = True
		else:
			self.listening = False

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
		msg = devices.MidiMessage(message)
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
		self.input.subscribe(self)

	def __eq__(self, other):
		return self.input.name == other.input.name and self.output.name == other.output.name

class Router:
	def __init__(self):
		self.connections = []
		self.listeners = {}
		self.inputs = {}
		self.outputs = {}
		self.load()

	def load(self):
		devices.load()

		self.inputs = {}
		self.outputs = {}
		for i in self.listeners:
			self.inputs[i] = -1
			self.outputs[i] = -1
			self.listeners[i].setup()

		self.inputs.update( devices.inputs )
		self.outputs.update( devices.outputs )

		for c in self.connections:
			c.connect()

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

class MidiLambda:
	def __init__(self, body):
		self.body = body
		self.func = eval("lambda x: " + self.body)
	def __call__(self, arg):
		return self.func(arg)
	def __str__(self):
		return self.body

class ChannelFilter(MidiLambda):
	def __init__(self, *channels):
		self.func = lambda x: x.getChannel() in channels
		self.body = "channel " + ",".join(map(str,channels))

def main(stdscr):
	router = Router()
	router.connect("Deluge", "Little Phatty SE II", ChannelFilter(3))
	router.connect("Deluge", "Circuit", ChannelFilter(0,1,2,10))
	router.connect("Deluge", "JUNO-DS", ChannelFilter(4,5,6,7,9,11))
	router.connect("JUNO-DS", "Circuit", ChannelFilter(1,2,10))
	router.connect("JUNO-DS", "Deluge", ChannelFilter(8))
	router.connect("JUNO-DS", "Little Phatty SE II", ChannelFilter(3))
	router.connect("BCR2000", "Little Phatty SE II", ChannelFilter(3))
	router.load()

	def hup(signum, frame):
		router.load()
		stdscr.clear()
	signal.signal(signal.SIGHUP, hup)

	ctrl = Controller(router)
	view = View(stdscr, router)

	while True:
		view.draw()
		view.set_cursor(ctrl.cursor)
		key = stdscr.getch()
		if key == ord('q'):
			break
		elif key == ord('r'):
			stdscr.clear()
		ctrl.key_press(key)

if __name__ == '__main__':
	#main(None)
	curses.wrapper(main)
