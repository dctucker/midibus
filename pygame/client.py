import socket
import select

class Client:
	def __init__(s):
		s.server_addr = ('127.0.0.1', 13949)
		s.client_addr = ('127.0.0.1', 13950)
		s.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		s.sock.setblocking(0)
		s.sock.bind(s.client_addr)
		s.callbacks = {
			'config': s.default_callback,
			'devices': s.default_callback,
		}
	def select(s):
		#result = select.select([s.sock],[],[], 0)
		#if s.sock in result[0]:
		s.listen()
	def listen(s):
		try:
			data, server = s.sock.recvfrom(1024)
		except socket.timeout:
			print('Response timeout')
			return
		except socket.error:
			print('Socket error')
			return
		callback_name = data.strip()
		response = []
		while True:
			try:
				data, server = s.sock.recvfrom(1024)
				if data == "\003":
					return s.callbacks[ callback_name ](response)
				response.append(data.strip())
			except socket.timeout:
				print('Unterminated response timeout')
	def ask(s, what):
		s.sock.sendto(what + "\n", s.server_addr)
	def get_config(s, callback):
		s.callbacks['config'] = callback
		s.ask('config')
	def get_devices(s, callback):
		s.callbacks['devices'] = callback
		s.ask('devices')
	def default_callback(s):
		pass

if __name__ == '__main__':
	print {
		'devices': Client().get_devices(),
		'config' : Client().get_config(),
	}
