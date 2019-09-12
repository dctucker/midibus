use std::sync::{Arc,RwLock};
use std::fmt;
use crate::filters::CallbackFn;
use crate::output::OutputDevice;
use crate::filters;

pub struct WriteData {
	pub output_device : Arc<RwLock<OutputDevice>>,
	pub func_name: String,
	args: String,
	//midi_in : Option<Arc<Mutex<Rawmidi>>>,
	pub callback : filters::Callback,
}
impl fmt::Debug for WriteData {
	fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
		write!(f, "WriteData: {{ {:#?}.{}({:?}) -> {:?} }}",
			self.output_device, self.func_name, self.args, self.callback)
	}
}

impl WriteData {
	pub fn new( out : Arc<RwLock<OutputDevice>>, func : String, args : String ) -> WriteData {
		WriteData {
			output_device: out.clone(),
			func_name: func.clone(),
			args: args.clone(),
			//midi_in: None,
			callback: filters::Callback::new( func, args.clone() ),
		}
	}
	pub fn add_args(&mut self) {
	}
	pub fn call(&mut self, buf : &Vec<u8>) {
		self.callback.callback(&mut self.output_device.write().unwrap(), buf);
	}
	/*
	pub fn update_midi_in(&mut self, midi_in : Arc<Mutex<Rawmidi>> ) {
		self.midi_in = Some(midi_in.clone());
	}
	*/
}
