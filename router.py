import sys
import rtmidi

class Devices:
	def __init__(self):
		self.inputs = {}
		self.open_inputs = {}
		self.open_outputs = {}
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

class Device:
	channels = [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16]
	def __init__(self):
		self.input = devices.input(self.name)
		self.input.setCallback( self )
		self.output = devices.output(self.name)
		self.subs = []

	def __call__(self, message):
		data = message.getRawData()
		print '\033[2K', 
		print ' '.join([hex(ord(d))[2:] for d in data]), "\r",
		sys.stdout.flush()
		self.forward(message)

	def forward(self, message):
		ch = message.getChannel()
		for sub in self.subs:
			if ch in sub.channels:
				sub.send(message)

	def send(self, message):
		self.output.sendMessage(message)

	def send_unison(self, message, channel):
		msg = rtmidi.MidiMessage(message)
		msg.setChannel(channel)
		self.output.sendMessage(msg)

	def subscribe(self, other):
		self.subs.append(other)

	def __rshift__(self, others):
		for other in others:
			self.subscribe(other)

class Deluge(Device):
	name = 'Deluge'

class BCR(Device):
	name = 'BCR2000'

class JUNO(Device):
	name = 'JUNO-DS'
	channels = [4,5,6,7,9,11]

class Circuit(Device):
	name = 'Circuit'
	channels = [0,1,2,10]

class Phatty(Device):
	name = 'Little Phatty SE II'
	channels = [3]
	#def send(self, message):
	#	self.send_unison(message, 3)

devices = Devices()
deluge = Deluge()
bcr = BCR()
juno = JUNO()
phatty = Phatty()
circuit = Circuit()

deluge >> juno, circuit, phatty
bcr    >> phatty,
juno   >> deluge, circuit, phatty


"""
matrix = (
	(deluge, (juno, circuit, phatty)),
	(bcr, (phatty,)),
	(juno, (deluge, circuit, phatty)),
)

for device, others in matrix:
	for other in others:
		device.subscribe(other)
"""

"""
deluge.subscribe(juno)
deluge.subscribe(circuit)
deluge.subscribe(phatty)

bcr.subscribe(phatty)

juno.subscribe(deluge)
juno.subscribe(phatty)
juno.subscribe(circuit)
"""





#while True:
#	message = device.getMessage(100)
#	call(message)


print 'HIT ENTER TO EXIT'
sys.stdin.read(1)

