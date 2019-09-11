use std::sync::{Arc,Mutex};
use std::fmt;
use crate::filters::CallbackFn;
use crate::output::OutputDevice;
use crate::filters;
use crate::lib::SafeRawmidi;
use alsa::rawmidi::Rawmidi;

pub struct WriteData {
	pub output_device : Arc<OutputDevice>,
	pub func_name: String,
	args: String,
	midi_in : Option<Arc<Mutex<Rawmidi>>>,
	pub callback : filters::Callback,
}
impl fmt::Debug for WriteData {
	fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
		write!(f, "WriteData: {{ {:#?}.{}({:?}) -> {:?} }}",
			self.output_device, self.func_name, self.args, self.callback)
	}
}

impl WriteData {
	pub fn new(out: Arc<OutputDevice>, func : String, args : String) -> WriteData {
		WriteData {
			output_device: out,
			func_name: func.clone(),
			args: args.clone(),
			midi_in: None,
			callback: filters::Callback::new( func, args.clone() ),
		}
	}
	pub fn add_args(&mut self) {
	}
	pub fn call(&self, buf : &Vec<u8>) {
		self.callback.callback(&self.output_device, buf);
	}
	pub fn update_midi_in(&mut self, midi_in : Arc<Mutex<Rawmidi>> ) {
		self.midi_in = Some(midi_in.clone());
	}
}
