from cffi import FFI

ffi = FFI()

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
ffi.set_source("app", None)

if __name__ == '__main__':
	ffi.compile()
