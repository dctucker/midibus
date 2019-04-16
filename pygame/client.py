import socket

class Client:
	def __init__(s):
		s.addr = ('127.0.0.1', 13949)
		s.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	def ask(s, what):
		s.sock.settimeout(1.0)
		s.sock.sendto(what + "\n", s.addr)
		while True:
			try:
				data, server = s.sock.recvfrom(1024)
				if data == "\003":
					break
				yield data
			except socket.timeout:
				print('REQUEST TIMED OUT')
	def get_config(s):
		return [r.strip() for r in s.ask('config')]
	def get_devices(s):
		return [r.strip() for r in s.ask('devices')]

if __name__ == '__main__':
	print {
		'devices': Client().get_devices(),
		'config' : Client().get_config(),
	}
