#!/usr/bin/env python2

import pygame, time, os, logging
from client import Client

os.putenv('SDL_VIDEODRIVER', 'fbcon')
os.putenv('SDL_FBDEV', '/dev/fb0')
os.putenv('SDL_MOUSEDRV', 'TSLIB')
os.putenv('SDL_MOUSEDEV', '/dev/input/event0')
 
# Init the pygame library and the display
pygame.init()
pygame.display.init()
DisplaySize = (pygame.display.Info().current_w, pygame.display.Info().current_h)
bgimage = pygame.image.load("bg.png")
bgrect  = bgimage.get_rect()
 
# Log the detected display size
logging.info(DisplaySize)
 
# Open fullscreen window
screen = pygame.display.set_mode(DisplaySize, pygame.NOFRAME)

class Octave:
	def __init__(s, x, y, ks=5, h=30):
		s.keysX = x
		s.keysY = y
		s.height = h
		s.keyspacing = ks
		s.state = [ 0,0,0,0,0, 0,0,0,0,0,0,0 ]

	def whitekey(s, col, color = (254,254,253) ):
		pygame.draw.rect(screen, color,      [s.keysX + s.keyspacing * col, s.keysY, 2 * s.keyspacing, s.height-1], 0)
		pygame.draw.line(screen, (75,75,75), [s.keysX + s.keyspacing * col, s.keysY], [s.keysX + s.keyspacing * col, s.keysY + s.height - 1], 1)
	def blackkey(s, ksm, color = (55,55,55) ):
		pygame.draw.rect(screen, color,      [1 + s.keysX + ksm * s.keyspacing, s.keysY, 1 + s.keyspacing, s.height / 2 ], 0)
		pygame.draw.rect(screen, (75,75,75), [1 + s.keysX + ksm * s.keyspacing, s.keysY, 1 + s.keyspacing, s.height / 2 ], 1)

	def color(s, n):
		if n in (0,2,4,5,7,9,11):
			if s.state[n] == 1:
				return (255,192,0)
			else:
				return (254,254,254)
		else:
			if s.state[n] == 1:
				return (192,128,0)
			else:
				return (55,55,55)
	def draw(s):
		s.whitekey(0, s.color(0))
		s.whitekey(2, s.color(2))
		s.whitekey(4, s.color(4))
		s.whitekey(6, s.color(5))
		s.whitekey(8, s.color(7))
		s.whitekey(10, s.color(9))
		s.whitekey(12, s.color(11))

		s.blackkey( 1.5, s.color(1))
		s.blackkey( 3.5, s.color(3))
		s.blackkey( 7.5, s.color(6))
		s.blackkey( 9.5, s.color(8))
		s.blackkey(11.5, s.color(10))
		pygame.draw.rect(screen, (75,75,75), [s.keysX, s.keysY, 1+s.keyspacing * 14, s.height], 1)

class KeysScreen:
	octs = []
	octs.append( Octave(125,   5) )
	octs.append( Octave(125,  45) )
	octs.append( Octave(125,  85) )
	octs.append( Octave(125, 125) )
	octs.append( Octave(125, 165) )
	octs.append( Octave(125, 205) )
	octs.append( Octave(125, 245) )
	octs.append( Octave(125, 285) )
	octs.append( Octave(285,   5) )
	octs.append( Octave(285,  45) )
	octs.append( Octave(285,  85) )
	octs.append( Octave(285, 125) )
	octs.append( Octave(285, 165) )
	octs.append( Octave(285, 205) )
	octs.append( Octave(285, 245) )
	octs.append( Octave(285, 285) )

	texts = []
	pygame.font.init()
	font = pygame.font.SysFont('Bitstream Vera Sans', 14)
	texts.append( font.render("Ch 1", True, (192,192,192)) )
	texts.append( font.render("Ch 2", True, (192,192,192)) )
	texts.append( font.render("Ch 3", True, (192,192,192)) )
	texts.append( font.render("Ch 4", True, (192,192,192)) )
	texts.append( font.render("Ch 5", True, (192,192,192)) )
	texts.append( font.render("Ch 6", True, (192,192,192)) )
	texts.append( font.render("Ch 7", True, (192,192,192)) )
	texts.append( font.render("Ch 8", True, (192,192,192)) )
	texts.append( font.render("Ch 9", True, (192,192,192)) )
	texts.append( font.render("Ch 10", True, (192,192,192)) )
	texts.append( font.render("Ch 11", True, (192,192,192)) )
	texts.append( font.render("Ch 12", True, (192,192,192)) )
	texts.append( font.render("Ch 13", True, (192,192,192)) )
	texts.append( font.render("Ch 14", True, (192,192,192)) )
	texts.append( font.render("Ch 15", True, (192,192,192)) )
	texts.append( font.render("Ch 16", True, (192,192,192)) )

	def draw(s):
		for o in s.octs:
			o.draw()
		for i in range(8):
			screen.blit(s.texts[i], (90, 10 + 40 * i))
			screen.blit(s.texts[8+i], (360, 10 + 40 * i))
		for y in range(8):
			pygame.draw.line(screen, (0,0,0), [0, y * 40], [480, y * 40], 1)

class Config:
	def __init__(s, client):
		s.devices = []
		s.connections = []
		s.connected = []
		s.client = client
		s.client.get_config(s.config_callback)
		s.client.select()
		s.client.get_devices(s.devices_callback)
		s.client.select()
	def config_callback(s, data):
		s.devices = []
		s.connections = []
		for line in data:
			i, o, f, a = line.split("\t")
			s.connections.append((i,o,f,a))
			if i not in s.devices:
				s.devices.append(i)
			if o not in s.devices:
				s.devices.append(o)
	def devices_callback(s, data):
		s.connected = data
	def get_matrix(s):
		ret = []
		for i,o,f,a in s.connections:
			ret.append((s.devices.index(i), s.devices.index(o)))
		return ret

def format_devname(d):
	return d.replace("hw:","")

class DeviceMatrix:
	def __init__(s, config):
		pygame.font.init()

		s.load_config(config)

	def load_config(s, config):
		s.config = config
		s.texts = []
		s.connections = s.config.get_matrix()
		s.cursor = [0,0]
		nd = len(s.config.devices)
		s.oy = s.ox = 25
		s.cy = s.cx = 200/nd
		font_size = 20 - max(0, min(8, (nd - 5)/2))
		s.font = pygame.font.SysFont('DejaVu Sans', int(font_size))
		s.small_font = pygame.font.SysFont('Inconsolata', 12)
		s.small_font.set_bold(True)
		for d in s.config.devices:
			color = (192,192,128) if d in s.config.connected else (32,32,64)
			s.texts.append( s.font.render( format_devname(d), True, color) )
		for d in s.config.devices:
			color = (192,192,128) if d in s.config.connected else (32,32,64)
			s.texts.append( pygame.transform.rotate( s.font.render( format_devname(d), True, color), 90) )

	def draw(s):
		l = len(s.config.devices)
		for i in range(l):
			is_connected = s.config.devices[i] in s.config.connected
			color = (128,128,96) if is_connected else (32,32,64)
			pygame.draw.line(screen, color, [0, s.get_y(i)], [s.get_x(l-1) + s.ox, s.get_y(i)], 1)
			pygame.draw.line(screen, color, [s.get_x(i), 0], [s.get_x(i), s.get_y(l-1) + s.oy], 1)
			half = s.texts[i].get_height() / 2
			screen.blit(s.texts[i],     (s.ox + s.get_x(l-1), s.get_y(i) - half))
			screen.blit(s.texts[l+i], (s.get_x(i) - half, s.oy + s.get_y(l-1)))
			if is_connected:
				s.draw_io_box('I', 0, s.get_y(i) - 6)
				s.draw_io_box('O', s.get_x(i) - 6, 0)
		for i,o in s.connections:
			is_connected = s.config.devices[i] in s.config.connected and s.config.devices[o] in s.config.connected
			color = (192,192,96) if is_connected else (128,128,128)
			pygame.draw.circle(screen, color, [s.get_x(o), s.get_y(i)], 5, 0 if is_connected else 1)
		s.draw_info_panel()

	def draw_info_panel(s):
		ix = 370
		screen.blit(bgimage, [ix, 200,210,120])

		conn = [x for x in s.config.connections if x[0] == s.config.devices[s.cursor[1]] and x[1] == s.config.devices[s.cursor[0]]]
		if len(conn) == 1:
			conn = conn[0]
			screen.blit(s.small_font.render(conn[2], True, (128,192,192)), [ix+10, 285])
			screen.blit(s.small_font.render(conn[3], True, (128,192,192)), [ix+10, 298])

		screen.blit(s.texts[s.cursor[1]], [ix+10, 230])
		screen.blit(s.texts[s.cursor[0]], [ix+10, 260])

		s.draw_io_box('I', ix-5, 235)
		s.draw_io_box('O', ix-5, 265)

		pygame.mouse.set_pos([matrix.get_x(matrix.cursor[0]), matrix.get_y(matrix.cursor[1])])
		pygame.mouse.set_cursor(*pygame.cursors.diamond)

	def draw_io_box(s, ch, x, y): 
		bg = (192,128,32)
		fg = (0,0,0)
		pygame.draw.rect(screen, bg, [x,y, 13,13], 0)
		screen.blit(s.small_font.render(ch , True, fg), [x+3,y-1])
	
	def get_x(s,o):
		return s.ox + s.cx * o
	def get_y(s,i):
		return s.oy + s.cy * i
	def set_cursor(s):
		x,y = pygame.mouse.get_pos()
		o = (x - s.ox + 0.5*s.cx) / s.cx
		i = (y - s.oy + 0.5*s.cy) / s.cy
		i = min(i, len(s.config.devices) - 1)
		i = int(max(i, 0))
		o = min(o, len(s.config.devices) - 1)
		o = int(max(o, 0))
		s.cursor = [o,i]
		s.draw_info_panel()


screen.blit(bgimage, bgrect)
#KeysScreen().draw()
client = Client()
matrix = DeviceMatrix(Config(client))
matrix.draw()
pygame.display.flip()

while True:
	# Wait for touchscreen touch events, until a keyboard key is pressed
	for event in pygame.event.get():
		if (event.type is pygame.MOUSEBUTTONDOWN):
			# Get touch position and write to log
			pos = pygame.mouse.get_pos()
			logging.info(pos)
		elif (event.type is pygame.MOUSEBUTTONUP):
			# Get finger lift position and write to log
			pos = pygame.mouse.get_pos()
			logging.info(pos)
			matrix.set_cursor()
			pygame.display.flip()
		elif (event.type is pygame.KEYDOWN):
			# Key pressed --> End program
			quit()

	client.select()

	time.sleep(0.2)
