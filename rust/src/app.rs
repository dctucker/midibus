use std::sync::Arc;
use std::collections::HashMap;
use crate::config::{Config};
use crate::thru::ReadThread;
use crate::thru::OutputDevice;
use crate::r#macro::MacroData;

type ReadThreadMap = HashMap<String,ReadThread>;
type OutputDeviceMap = HashMap<String,Arc<OutputDevice>>;

#[derive(Debug)]
pub struct App {
	pub read_threads : ReadThreadMap,
	pub output_devices : OutputDeviceMap,
	pub config : Config,
	pub macro_data : Vec<MacroData>,
}

impl App {
	pub fn new() -> App {
		let config = Config::new();
		let read_threads = HashMap::new();
		let output_devices = HashMap::new();

		let mut app = App {
			config : config,
			read_threads : read_threads,
			output_devices : output_devices,
			macro_data : vec![],
		};
		app.init_output_devices();
		app.init_read_threads();
		app.setup_read_threads();
		//app.setup_output_devices();
		app
	}

	fn init_output_devices(&mut self) {
		for key in self.config.uniq_out() {
			if ! self.output_devices.contains_key(&key) {
				self.output_devices.insert(key.clone(), Arc::new(OutputDevice::new(key.clone())));
			}
		}
	}

	fn init_read_threads(&mut self) {
		for key in self.config.uniq_in() {
			if ! self.read_threads.contains_key(&key) {
				self.read_threads.insert(key.clone(), ReadThread::new(key.clone()));
			}
		}
	}

	fn setup_read_threads(&mut self) {
		let lines = self.config.lines();
		for l in lines {
			let line = l.clone();
			let key = line.r#in.to_string();
			let out = self.output_devices.get(&line.out).unwrap();
			self.read_threads.get_mut(&key).unwrap()
				.setup_write(
					Arc::clone(out),
					line.func.to_string(),
					line.args.to_string()
				);
		}
	}

	pub fn join(self) {
		for (name,thread) in self.read_threads {
			thread.join();
		}
	}

	pub fn add_macro(&mut self) {
		self.macro_data.push( MacroData {
			name: "Nothing".to_string(),
			data: vec![]
		});
	}
}
