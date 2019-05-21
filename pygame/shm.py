import mmap
from cffi import FFI
from posix_ipc import SharedMemory

class Struct:
	def __init__(s, data):
		s.data = data
	def __repr__(s):
		ret = str(s.data) + " "
		d = {}
		for k in dir(s.data):
			v = getattr(s.data,k)
			if v == ffi.NULL:
				v = None
			elif isinstance(v, ffi.CData) and ffi.typeof(v).cname.startswith('char'):
				v = ffi.string(v)
			d[k] = v
		ret += str(d)
		return ret

class Array:
	def __init__(s, data):
		s.data = data
	def __repr__(s):
		a = []
		ret = str(s.data) + " "
		for v in s.data:
			if isinstance(v, ffi.CData):
				cname = ffi.typeof(v).cname
				if cname.startswith('char *'):
					v = CharStar(v)
				elif cname.startswith('char'):
					v = ffi.string(v)
			a += [v]
		ret += str(a)
		return ret

class CharStar:
	def __init__(s, data):
		s.data = data
	def __str__(s):
		if s.data != ffi.NULL:
			s = pointer + int(ffi.cast("intptr_t", s.data)) - app.base
			return ffi.string(s)
	def __repr__(s):
		if s.data == ffi.NULL:
			return 'None'
		return str(s).__repr__()

ffi = FFI()

def setup_ffi():
	headers = ""
	for obj in ['common','thru','write','app','macro']:
		with open('../server/'+obj+'.h') as f:
			header = ""
			for line in f.readlines():
				if line.startswith("#include "):
					continue
				if line.startswith("#define MASK_"):
					continue
				header += line + "\n"
			headers += header
	#pthread_t from arm-linux-gnueabihf/bits/pthreadtypes.h
	ffi.cdef("""
	typedef unsigned long int pthread_t;
	typedef struct _snd_rawmidi snd_rawmidi_t;
	""" + headers)
	#ffi.set_source("_app", '#include "../server/app.h"')

setup_ffi()
shm = SharedMemory("/midi-server", 0, 0644, 0, True)
mem = mmap.mmap(shm.fd, shm.size, mmap.MAP_SHARED, mmap.PROT_READ)

pointer = ffi.from_buffer(mem)
app = ffi.cast("struct app_t *", pointer)

