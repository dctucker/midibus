use std::rc::Rc;
use std::fmt;
use alsa::rawmidi::Rawmidi;
use crate::thru::OutputDevice;
use crate::filters;

pub struct WriteData {
	pub output_device : Rc<OutputDevice>,
	pub func_name: String,
	args: String,
	midi_in : Option<Rawmidi>,
	callback : filters::Callback,
}
impl fmt::Debug for WriteData {
	fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
		write!(f, "WriteData: {{ output_device: {:#?}, func_name: {}, args: {:?}, callback: {:?} }}",
			self.output_device, self.func_name, self.args, self.callback)
	}
}

impl WriteData {
	pub fn new(out: Rc<OutputDevice>, func : String, args : String) -> WriteData {
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
