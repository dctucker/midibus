extern crate alsa;

use std::rc::Rc;
use std::fmt;
use std::thread;
use std::thread::JoinHandle;
use std::time::Duration;
use alsa::rawmidi::Rawmidi;
use crate::r#macro::MacroListener;
use crate::write::WriteData;

pub struct OutputDevice {
    midi : Option<Rawmidi>,
    status : u8,
    midi_in_exclusive : Option<Rawmidi>,
    port_name : String
}

impl fmt::Debug for OutputDevice {
	fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
		write!(f, "OutputDevice {{ port_name: {} }}", self.port_name)
	}
}

impl OutputDevice {
	pub fn new(name : String) -> OutputDevice {
		OutputDevice {
			port_name: name,
			status: 0,
			midi: None,
			midi_in_exclusive: None,
		}
	}
}


pub struct ReadThread {
	handle : JoinHandle<()>,
    midi : Option<Rawmidi>,
    port_name : String,
    outs : Vec<WriteData>,
    macros : Vec<MacroListener>
}

impl ReadThread {
	pub fn new(port_name: String) -> ReadThread {
		let name = port_name.clone();
		let handle = thread::spawn(move || {
			println!("{}", name);
			thread::sleep(Duration::from_millis(10000));
			println!("{} done", name);
		});
		ReadThread {
			handle: handle,
			midi: None,
			port_name: port_name,
			outs: vec![],
			macros: vec![],
		}
	}
	pub fn join(self) {
		self.handle.join().unwrap();
	}
	pub fn attach(&mut self, out : Rc<OutputDevice>, func : String, args : String) {
		self.outs.push( WriteData::new(out, func, args) );
		//println!("{:?}", self.outs);
	}
}
