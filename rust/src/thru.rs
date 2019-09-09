extern crate alsa;

use std::cell::{RefCell,Cell};
use std::sync::{Mutex,RwLock};
use std::sync::Arc;
use std::fmt;
use std::thread;
use std::thread::JoinHandle;
use std::time::Duration;
use alsa::rawmidi::Rawmidi;
use crate::r#macro::MacroListener;
use crate::write::WriteData;

pub struct OutputDevice {
	midi : Option<Mutex<Rawmidi>>,
	status : u8,
	midi_in_exclusive : Option<Mutex<Rawmidi>>,
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

pub struct ReadData {
	midi : Option<Mutex<Rawmidi>>,
	port_name : String,
	outs : Vec<WriteData>,
	macros : Vec<MacroListener>
}
impl fmt::Debug for ReadData {
	fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
		write!(f, "ReadData {{ port_name: {}, outs: {:#?}, macros: {:#?} }}", self.port_name, self.outs, self.macros)
	}
}
impl ReadData {
	pub fn new( name : String ) -> ReadData {
		ReadData {
			midi: None,
			port_name: name,
			outs: vec![],
			macros: vec![],
		}
	}
	pub fn setup_write(&mut self, out : Arc<OutputDevice>, func : String, args : String) {
		for mut wd in &mut self.outs {
			if wd.func_name == func && wd.output_device.port_name == out.port_name {
				wd.add_args();
				return
			}
		}
		self.outs.push( WriteData::new(out, func, args) );
	}
}

pub struct ReadThread {
	handle : JoinHandle<()>,
	data : Arc<RwLock<ReadData>>,
}
impl fmt::Debug for ReadThread {
	fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
		write!(f, "ReadThread {{ data: {:#?} }}", self.data)
	}
}
impl ReadThread {
	pub fn new(port_name: String) -> ReadThread {
		let data = RwLock::new(ReadData::new(port_name));
		let arc = Arc::new(data);
		ReadThread {
			data: arc.clone(),
			handle: thread::spawn(move || {
				let data = arc.read().unwrap();
				println!("{}", data.port_name);
				thread::sleep(Duration::from_millis(1000));
				println!("{} done", data.port_name);
			}),
		}
	}
	pub fn join(self) {
		self.handle.join().unwrap();
	}
	pub fn run(&self) {
	}

	pub fn setup_write(&mut self, out : Arc<OutputDevice>, func : String, args : String) {
		let mut data = self.data.write().unwrap();
		data.setup_write(out,func,args);
	}
}
