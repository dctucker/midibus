use std::sync::{Arc,RwLock};
use std::sync::atomic::{AtomicBool, Ordering};
use std::collections::HashMap;
use crate::config::{Config};
use crate::thru::ReadThread;
use crate::output::OutputDevice;
use crate::r#macro::MacroData;

type ReadThreadMap = HashMap<String,ReadThread>;
type OutputDeviceMap = HashMap<String,Arc<RwLock<OutputDevice>>>;

#[derive(Debug)]
pub struct App {
	pub read_threads : ReadThreadMap,
	pub output_devices : OutputDeviceMap,
	pub config : Config,
	pub macro_data : Vec<MacroData>,
	pub hup : Arc<AtomicBool>,
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
			hup : Arc::new(AtomicBool::new(false)),
		};
		app.init_output_devices();
		app.init_read_threads();
		app.setup_read_threads();
		//app.setup_output_devices();
		println!("app setup complete");
		app
	}

	fn init_output_devices(&mut self) {
		println!("setup output threads");
		for key in self.config.uniq_out() {
			if key == "server" || key == "macro" { continue; }
			if ! self.output_devices.contains_key(&key) {
				self.output_devices.insert(key.clone(),
					Arc::new(RwLock::new(OutputDevice::new(key.clone())))
				);
			}
		}
	}

	fn init_read_threads(&mut self) {
		println!("init read threads");
		for key in self.config.uniq_in() {
			if key == "server" || key == "macro" { continue; }
			if ! self.read_threads.contains_key(&key) {
				let read_thread = ReadThread::new(key.clone());
				self.read_threads.insert(key.clone(), read_thread);
			}
		}
	}

	fn setup_read_threads(&mut self) {
		println!("setup read threads");
		let lines = self.config.lines();
		for line in lines.iter() {
			println!("Setting up line {:?}", line);
			let key = line.r#in.to_string();
			if key == "server" || key == "macro" { continue; }
			match self.output_devices.get(&line.out) {
				Some(out) => {
					self.read_threads.get(&key).unwrap()
						.setup_write(
							out.clone(),
							line.func.to_string(),
							line.args.to_string()
						);
				},
				None => {
					println!("Output device not found {}", line.out);
				},
			}
			println!("setup_write done");
		}
		println!("setup read return");
	}

	pub fn join(&mut self) {
		println!("joining");
		loop {
			if self.hup.swap(false, Ordering::Relaxed) {
				println!("Got SIGHUP");
				for (_name,thread) in self.read_threads.iter_mut() {
					thread.hup.store(true, Ordering::Relaxed);
				}
			}
		}
		/*
		for (_name,thread) in self.read_threads {
			thread.join();
		}
		*/
	}

	/*
	pub fn add_macro(&mut self) {
		self.macro_data.push( MacroData {
			name: "Nothing".to_string(),
			data: vec![]
		});
	}
	*/
}
