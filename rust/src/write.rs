use std::rc::Rc;
use std::fmt;
use alsa::rawmidi::Rawmidi;
use crate::thru::OutputDevice;

#[derive(Debug)]
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

#[derive(Debug)]
struct WriteCallback {
	name : String,
	args : Option<WriteArgs>,
}

trait WriteCallbackFn {
	fn callback(write_data : WriteData, buf : &Vec<u8>) -> usize;
}

impl WriteCallbackFn for WriteCallback {
	fn callback(write_data : WriteData, buf : &Vec<u8>) -> usize { 0 }
}

pub struct WriteData {
	pub output_device : Rc<OutputDevice>,
	pub func_name: String,
	args: String,
	midi_in : Option<Rawmidi>,
	callback : WriteCallback,
}
impl fmt::Debug for WriteData {
	fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
		write!(f, "WriteData: {}", self.func_name)
	}
}

impl WriteData {
	pub fn new(out: Rc<OutputDevice>, func : String, args : String) -> WriteData {
		WriteData {
			output_device: out,
			func_name: func.clone(),
			args: args,
			midi_in: None,
			callback: WriteCallback { name: func.clone(), args: None },
		}
	}
	pub fn add_args(&mut self) {
	}
}
