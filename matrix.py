#!/usr/bin/env python2
# -*- coding: utf-8 -*-

import signal
import locale
locale.setlocale(locale.LC_ALL, '')

import curses
import alsaseq

alsaseq.client("Matrix", 1, 1, False)

class Model:
	def __init__(self):
		self.load()

	def load(self):
		devices = alsaseq.listdevices()

		inputs = []
		outputs = []
		devs = {}

		for d in devices:
			if devices[d]["num"] <= 1:
				continue
			num = devices[d]["num"]
			name = devices[d]["name"]
			devs[num] = name
			for p in devices[d]["ports"]:
				caps = p["caps"]
				if caps & 33L: # output
					inputs += [{
						"device": {
							"num": num,
							"name": name,
						},
						"port": p,
					}]
				if caps & 66L: # input
					outputs += [{
						"device": {
							"num": num,
							"name": name,
						},
						"port": p,
					}]
		self.inputs = inputs
		self.outputs = outputs
		self.devices = devs

		self.load_connections()
	def load_connections(self):
		self.connections = alsaseq.listconnections()
	
	def get_index_in(self, num):
		j = 0
		for i in self.inputs:
			if i["port"]["num"] == num[1] and i["device"]["num"] == num[0]:
				return j
			j += 1
		return False

	def get_index_out(self, num):
		j = 0
		for i in self.outputs:
			if i["port"]["num"] == num[1] and i["device"]["num"] == num[0]:
				return j
			j += 1
		return False

	def short_name(self,i):
		#name = i["port"]["name"]
		#if not i["device"]["name"] in i["port"]["name"]:
		#	name = i["device"]["name"] + ": " + name
		#return name
		return i["device"]["name"] + ":" + str(i["port"]["num"])

	def connect(self, i, o):
		alsaseq.connect( i["device"]["num"], i["port"]["num"], o["device"]["num"], o["port"]["num"] )

	def disconnect(self, i, o):
		alsaseq.disconnect( i["device"]["num"], i["port"]["num"], o["device"]["num"], o["port"]["num"] )


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
			self.model.load_connections()
		elif c in (ord("\b"),curses.KEY_BACKSPACE):
			i = self.model.inputs[ self.cursor[0] ]
			o = self.model.outputs[ self.cursor[1] ]
			self.model.disconnect(i,o)
			self.model.load_connections()


class View:
	def __init__(self, screen, model):
		self.screen = screen
		self.cursor = [0,0]
		self.model = model

	def input_left(self):
		return len(self.model.inputs) * 2

	def output_top(self):
		return len(self.model.inputs) + 1

	def draw(self):
		model = self.model
		s = self.screen
		y = 1
		x = self.input_left()
		for i in model.inputs:
			s.addstr(y, x, model.short_name(i))
			y += 1
		max_y = y
		y += len(model.outputs)
		#y = 1
		x = 0
		for o in model.outputs:
			s.addstr(y, x, model.short_name(o))
			y -= 1
			x += 2
		
		y = self.output_top()
		for x in range(len(model.outputs)):
			s.move(y, x * 2)
			s.vline(curses.ACS_VLINE, len(model.outputs) - x)

		for y in range(len(model.inputs)):
			for x in range(len(model.outputs)):
				s.move(y+1, x * 2)
				s.addch(curses.ACS_PLUS)
				s.addch(curses.ACS_HLINE)

		for p1, p2 in model.connections:
			y = 1 + model.get_index_in(p1)
			x = model.get_index_out(p2) * 2
			s.move(y,x)
			s.addch(curses.ACS_DIAMOND)
			#s.addstr(y, x, "╆")

	def set_cursor(self, cursor):
		s = self.screen
		s.chgat( 1+cursor[0], self.input_left(), len( self.model.short_name( self.model.inputs[cursor[0]] ) ), curses.A_REVERSE )
		s.chgat( len( self.model.outputs ) - cursor[1] + self.output_top(), 2*cursor[1], len( self.model.short_name( self.model.outputs[cursor[1]] ) ), curses.A_REVERSE )
		self.cursor = cursor
		self.screen.move( 1+cursor[0], cursor[1] * 2 )


	
def main(stdscr):
	model = Model()
	ctrl = Controller(model)
	view = View(stdscr, model)

	def hup(signum, frame):
		ctrl.reload()
		stdscr.clear()
		view.draw()
	signal.signal(signal.SIGHUP, hup)

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
	curses.wrapper(main)
