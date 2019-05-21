from shm import ffi

class Config:
	def __init__(s):
		s.devices = []
		s.connections = []
		s.connected = []

	def get_matrix(s):
		ret = []
		for i,o,f,a in s.connections:
			ret.append((s.devices.index(i), s.devices.index(o)))
		return ret

class ClientConfig(Config):
	def __init__(s, client):
		Config.__init__(s)
		s.client = client
		s.load()

	def load(s):
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

def strattr(line, x):
	return ffi.string(getattr(line, x))

class ShmConfig(Config):
	def __init__(s, app):
		Config.__init__(s)
		s.app = app
		s.load()

	def load(s):
		s.devices = []
		ins  = [ strattr(line, 'in')  for line in s.app.config ]
		outs = [ strattr(line, 'out') for line in s.app.config ]
		all_devs = filter(lambda x: x not in ('','server'), ins + outs)
		[s.devices.append(x) for x in all_devs if x not in s.devices]

		s.connections = filter(lambda x: x != ('','','',''), [ tuple([ffi.string(v) for v in line.line]) for line in s.app.config])
		s.connected = [str(CharStar(d.port_name)) for d in s.app.read_data if d.midi != ffi.NULL]

