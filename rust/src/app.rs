use std::thread::Thread;
use crate::config::{ConfigLine,load_configuration};
use crate::thru::ReadThreadData;
use crate::thru::OutputDevice;
use crate::r#macro::MacroData;

pub struct App {
	threads : Vec<Thread>,
	pub read_data : Vec<ReadThreadData>,
	pub output_devices : Vec<OutputDevice>,
	pub config_lines : Vec<ConfigLine>,
	pub macro_data : Vec<MacroData>,
}

impl App {
	pub fn new() -> App {
		App {
			config_lines : load_configuration(),
			threads : vec![],
			read_data : vec![],
			output_devices : vec![],
			macro_data : vec![],
		}
	}
	pub fn add_macro(&mut self) {
		self.macro_data.push( MacroData {
			name: "Nothing".to_string(),
			data: vec![]
		});
	}
}
