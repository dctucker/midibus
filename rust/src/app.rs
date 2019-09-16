use std::sync::Arc;
use std::sync::atomic::{AtomicBool, Ordering};
use crate::config::{Config,ConfigLine};
use crate::r#macro::MacroData;
use crate::devices::{ReadThreadMap,OutputDeviceMap};
use std::time::Duration;
use std::thread;

#[derive(Debug)]
pub struct Flags {
	pub hup : Arc<AtomicBool>,
	pub int : Arc<AtomicBool>,
	pub run : Arc<AtomicBool>,
}
impl Flags {
	pub fn new() -> Flags {
		Flags {
			hup : Flags::newflag(false),
			int : Flags::newflag(false),
			run : Flags::newflag(false),
		}
	}
	fn newflag( val : bool ) -> Arc<AtomicBool> { Arc::new( AtomicBool::new( val ) ) }
}

#[derive(Debug)]
pub struct App {
	pub read_threads : ReadThreadMap,
	pub output_devices : OutputDeviceMap,
	pub config : Config,
	pub macro_data : Vec<MacroData>,
	pub flags : Arc<Flags>,
}

impl App {
	pub fn new() -> App {
		let config = Config::new();
		let read_threads = ReadThreadMap::new();
		let output_devices = OutputDeviceMap::new();

		let mut app = App {
			config: config,
			read_threads: read_threads,
			output_devices: output_devices,
			macro_data: vec![],
			flags: Arc::new( Flags::new() ),
		};
		if false {
			app.init_output_devices();
			app.init_read_threads();
		}
		app.read_config_lines();
		app.ready();
		//app.setup_output_devices();
		println!("app setup complete");
		app
	}

	fn init_read_threads(&mut self) {
		for key in self.config.uniq_in() {
			self.read_threads.by_name(&key);
		}
	}

	fn init_output_devices(&mut self) {
		for key in self.config.uniq_out() {
			self.output_devices.by_name(&key);
		}
	}

	fn read_config_lines(&mut self) {
		for line in self.config.lines().iter() {
			App::setup_line(line, &mut self.read_threads, &mut self.output_devices);
		}
	}

	fn setup_line( line : &ConfigLine, read_threads : &mut ReadThreadMap, output_devices : &mut OutputDeviceMap ) {
		print!("Setting up line {:?} ", line);
		let key = line.r#in.to_string();
		if key == "server" || key == "macro" { println!("TODO implement {} support", key); return; }
		if line.out == "server" || line.out == "macro" { println!("TODO implement {} support", line.out); return; }
		println!();
		let read = &read_threads.by_name(&line.r#in);
		let out = &output_devices.by_name(&line.out);
		read.setup_write( &out, line.func.to_string(), line.args.to_string());
	}

	pub fn ready(&mut self) {
		self.flags.run.store(true, Ordering::Relaxed);
		for (_name,thread) in self.read_threads.iter_mut() {
			thread.flags.run.store(true, Ordering::Relaxed);
		}
	}

	pub fn join(&mut self) {
		println!("App thread signal yield loop");
		'signal: loop {
			if self.flags.hup.swap(false, Ordering::Relaxed) {
				println!("Got SIGHUP");
				for (_name,thread) in self.read_threads.iter_mut() {
					thread.flags.hup.store(true, Ordering::Relaxed);
				}
			}
			if self.flags.int.swap(false, Ordering::Relaxed) {
				println!("Got SIGINT");
				for (_name,thread) in self.read_threads.iter_mut() {
					thread.flags.int.store(true, Ordering::Relaxed);
				}
				break 'signal;
			}
			thread::sleep(Duration::from_millis(500));
		}
		
		let mut done = false;
		while ! done {
			done = true;
			for (_name,thread) in self.read_threads.iter() {
				if thread.flags.int.load(Ordering::Relaxed) {
					done = false;
					//println!("{} not done", _name);
					std::thread::yield_now();
					thread::sleep(Duration::from_millis(500));
					break;
				}
			}
		}
		//println!("Finishing up");
		//self.read_threads.join_all();
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
impl Drop for App {
	fn drop(&mut self) {
		println!("Bye!");
	}
}
