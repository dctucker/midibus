use std::rc::Rc;
use alsa::rawmidi::Rawmidi;
use crate::thru::OutputDevice;

enum WriteArgs {
	ChannelFilterData {
		mask : u32,
	},
	FunnelFilterData {
		channel : u8,
	},
	CCMapFilterData {
		channel : u8,
		out_cc : Vec<u8>,
	},
	CCMapStatusData {
		out_status : Vec<u8>,
	},
}

struct WriteCallback {
	args : WriteArgs,
	out_buf : Vec<u8>,
}

trait WriteCallbackFn {
	fn callback(write_data : WriteData, buf : &Vec<u8>) -> usize;
}

impl WriteCallbackFn for WriteCallback {
	fn callback(write_data : WriteData, buf : &Vec<u8>) -> usize { 0 }
}

#[derive(Debug)]
pub struct WriteData {
	output_device : Rc<OutputDevice>,
	func: String,
	args: String,
	/*
	midi_in : Option<Rawmidi>,
	callbacks : Vec<WriteCallback>,
	*/
}

impl WriteData {
	pub fn new(out: Rc<OutputDevice>, func : String, args : String) -> WriteData {
		WriteData {
			output_device: out,
			func: func,
			args: args,
		}
	}
}
