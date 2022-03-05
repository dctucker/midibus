#? replace(sub = "\t", by = " ")

from posix import onSignal, SIGINT, SIGHUP
import common
import socket
import app

type
	ThreadData = object
		id: int
		message: string
	MyThread = Thread[ThreadData]

const MAX_THREADS = 16
var threads = newSeqOfCap[MyThread](MAX_THREADS)
var stop_all = false

proc handler(data: ThreadData) =
  echo data.id, " ", data.message

proc load_config_file() =
	todo "load config file"
proc manage_inputs() =
	todo "manage inputs"
proc manage_outputs() =
	todo "manage outputs"

proc sighup_handler() =
	var hanging_up {.global.} = false
	if hanging_up: return
	hanging_up = true

	stdout.write("M Reloading\n")
	stdout.flushFile()
	load_config_file()

	manage_inputs()
	manage_outputs()
	emit_devices()

	stdout.flushFile()
	hanging_up = false

proc sigint_handler() =
	stop_all = true
	stdout.write("\nM interrupt\n")

proc main() =
	init_app()

	onSignal(SIGINT):
	  sigint_handler()
	onSignal(SIGHUP):
	  sighup_handler()

	#for i in 0..<MAX_THREADS:
	#  var data = ThreadData(id: i, message: "hello world")
	#  threads.add(MyThread())
	#  createThread(threads[i], handler, data)
	#joinThreads(threads)

when isMainModule:
	main()
