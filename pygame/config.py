
class Config:
	def __init__(s, client):
		s.devices = []
		s.connections = []
		s.connected = []
		s.client = client

		#s.devices = [CharStar(d.port_name) for d in app.read_data if d.midi != ffi.NULL]

		s.client.callbacks['config'] = s.config_callback
		s.client.get_config()
		s.client.select()

		s.client.callbacks['devices'] = s.devices_callback
		s.client.get_devices()
		s.client.select()

	def config_callback(s, data):
		s.devices = []
		s.connections = []
		for line in data:
			i, o, f, a = line
			s.connections.append((i,o,f,a))
			if i not in s.devices:
				s.devices.append(i)
			if o not in s.devices:
				s.devices.append(o)
		print s.devices
		print s.connections
	def devices_callback(s, data):
		s.connected = [ d[0] for d in data ]
		print s.connected
	def get_matrix(s):
		ret = []
		for i,o,f,a in s.connections:
			ret.append((s.devices.index(i), s.devices.index(o)))
		return ret
