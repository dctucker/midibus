from rtmidi import RtMidiIn,RtMidiOut,MidiMessage

inputs = {}
outputs = {}
open_inputs = {}
open_outputs = {}

main_out = RtMidiOut()
main_in = RtMidiIn()

def load():
	ins()
	outs()

def ins():
	global inputs
	inputs_ = {}
	for i in range(main_in.getPortCount()):
		name = ' '.join(main_in.getPortName(i).split(' ')[:-1])
		inputs_[ name ] = i
	inputs = inputs_

def outs():
	global outputs
	outputs_ = {}
	for i in range(main_out.getPortCount()):
		name = ' '.join(main_out.getPortName(i).split(' ')[:-1])
		outputs_[ name ] = i
	outputs = outputs_


def input(name):
	global open_inputs
	if name not in inputs.keys() or len(name) == 0:
		raise Exception("Device '%s' not found" % name)
	if name not in open_inputs.keys():
		rt = RtMidiIn()
		rt.openPort( inputs[name] )
		#rt.ignoreTypes(False, False, False)
		open_inputs[name] = rt
	return open_inputs[name]

def output(name):
	global open_outputs
	if name not in outputs.keys() or len(name) == 0:
		raise Exception("Device '%s' not found" % name)
	if name not in open_outputs.keys():
		rt = RtMidiOut()
		rt.openPort( outputs[name] )
		open_outputs[name] = rt
	return open_outputs[name]

import atexit
def cleanup():
	global open_outputs, open_inputs
	for rt in open_outputs.values():
		rt.closePort()
	for rt in open_inputs.values():
		rt.closePort()
atexit.register(cleanup)

load()
