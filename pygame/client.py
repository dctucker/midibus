import socket
import select
import sys

BUFFER_SIZE=4096

class Client:
	def __init__(s):
		#s.sun_path = "../midi-server.sock"
		s.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		s.server_socket.connect(('127.0.0.1', 13949))
		s.callbacks = {
			'config' : s.default_callback,
			'devices': s.default_callback,
		}

	def disconnect(s):
		s.ask("bye");

	def __del__(s):
		pass

	def select(s):
		readable, wrable, excep = select.select([s.server_socket],[],[s.server_socket], 0.2)
		if s.server_socket in excep:
			print "WHOA"
		if s.server_socket in readable:
			return s.listen()

	def listen(s):
		try:
			response = [ l.split("\t") for l in s.server_socket.recv(BUFFER_SIZE).strip().split("\n") ]
		except socket.timeout:
			print('Response timeout')
			return
		except socket.error:
			print('Socket error')
			return
		s.callback_name = response[0][0]
		print "calling " +  s.callback_name
		s.callbacks[ s.callback_name ](response[1:])
		return True
	def ask(s, what):
		s.server_socket.send(what + "\n")
	def get_config(s):
		s.ask('config')
	def get_devices(s):
		s.ask('devices')
	def default_callback(s, data):
		print data
		pass

if __name__ == '__main__':
	def callback(data):
		print data

	c = Client()
	c.callbacks['config'] = callback
	c.get_config()
	c.select()

	c.callbacks['devices'] = callback
	c.get_devices()
	c.select()

	c.disconnect()

