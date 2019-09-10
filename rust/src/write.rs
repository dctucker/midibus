use std::sync::{Arc,Mutex};
use std::fmt;
use crate::output::OutputDevice;
use crate::filters;
use crate::lib::SafeRawmidi;

pub struct WriteData {
	pub output_device : Arc<OutputDevice>,
	pub func_name: String,
	args: String,
	midi_in : SafeRawmidi,
	callback : filters::Callback,
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
}
