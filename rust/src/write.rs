use std::sync::{Arc,RwLock};
use std::fmt;
use crate::filters::CallbackFn;
use crate::output::OutputDevice;
use crate::filters;
use crate::thru::BUFSIZE;

pub struct CallbackData<'a> {
	pub output_device : &'a mut OutputDevice,
	pub buffer : &'a mut [u8],
	pub midi_in : String,
}

pub struct WriteData {
	pub output_device : Arc<RwLock<OutputDevice>>,
	pub func_name: String,
	args: String,
	pub midi_in : String,
	pub callback : filters::Callback,
	pub buffer : [u8 ; BUFSIZE],
}
impl fmt::Debug for WriteData {
	fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
		write!(f, "WriteData: {{ {:#?}.{}({:?}) -> {:?} }}",
			self.output_device, self.func_name, self.args, self.callback)
	}
}

impl WriteData {
	pub fn new( out : &Arc<RwLock<OutputDevice>>, func : String, args : String ) -> WriteData {
		WriteData {
			output_device: out.clone(),
			func_name: func.clone(),
			args: args.clone(),
			midi_in: "".to_string(),
			buffer: [ 0u8 ; BUFSIZE ],
			callback: filters::Callback::new( func, args.clone() ),
		}
	}
	pub fn add_args(&mut self, args : String) {
		self.callback.add_args(args);
	}
	pub fn call(&mut self, buf : &[u8]) {
		let output_device = &mut match self.output_device.write() {
			Ok(dev) => dev,
			Err(_) => {
				return;
			},
		};
		let mut data = CallbackData {
			output_device: output_device,
			buffer: &mut self.buffer,
			midi_in: self.midi_in.to_string(),
		};
		self.callback.callback(&mut data, buf);
		self.midi_in = data.midi_in.to_string();
	}
	/*
	pub fn update_midi_in(&mut self, midi_in : &String ) {
		self.midi_in = midi_in.clone();
	}
	*/
}
