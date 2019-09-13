use std::sync::{Arc,RwLock};
use std::sync::atomic::{AtomicBool, Ordering};
use std::collections::HashMap;
use crate::config::{Config,ConfigLine};
use crate::thru::ReadThread;
use crate::output::OutputDevice;
use crate::r#macro::MacroData;

//pub type ReadThreadMap = HashMap<String,ReadThread>;
//pub type OutputDeviceMap = HashMap<String,Arc<RwLock<OutputDevice>>>;

#[derive(Debug)]
pub struct ReadThreadMap( HashMap<String,ReadThread> );
impl ReadThreadMap {
	pub fn new() -> ReadThreadMap { ReadThreadMap( HashMap::new() ) }
	pub fn by_name( &mut self, key : &String ) -> &ReadThread {
		if ! self.0.contains_key(key) {
			println!("New input device {}", key);
			let read_thread = ReadThread::new(String::from(key));
			self.0.insert(String::from(key), read_thread);
		}
		self.0.get(key).unwrap()
	}
}

#[derive(Debug)]
pub struct OutputDeviceMap( HashMap<String,Arc<RwLock<OutputDevice>>> );
impl OutputDeviceMap {
	pub fn new() -> OutputDeviceMap { OutputDeviceMap( HashMap::new() ) }
	pub fn by_name( &mut self, key : &String ) -> &Arc<RwLock<OutputDevice>> {
		if ! self.0.contains_key(key) {
			println!("New output device {}", key);
			self.0.insert( String::from(key),
				Arc::new(RwLock::new(OutputDevice::new(key.clone())))
			);
		}
		self.0.get(key).unwrap()
	}
}


#[derive(Debug)]
pub struct Flags {
	pub hup : Arc<AtomicBool>,
	pub run : Arc<AtomicBool>,
}
impl Flags {
	pub fn new() -> Flags {
		Flags {
			hup : Flags::newflag(false),
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
		//app.init_output_devices();
		//app.init_read_threads();
		app.read_config_lines();
		app.ready();
		//app.setup_output_devices();
		println!("app setup complete");
		app
	}

	/*
	pub fn init_output_device(&mut self, key : &String) {
		if key == "server" || key == "macro" { return; }
		if ! self.output_devices.contains_key(key) {
			println!("New output device {}", key);
			self.output_devices.insert( String::from(key),
				Arc::new(RwLock::new(OutputDevice::new(key.clone())))
			);
		}
	}

	fn init_output_devices(&mut self) {
		for key in self.config.uniq_out() {
			self.init_output_device(&key);
		}
	}

	fn init_read_thread(&mut self, key : &String) -> &ReadThread {
		if ! self.read_threads.contains_key(key) {
			println!("New input device {}", key);
			let read_thread = ReadThread::new(String::from(key));
			self.read_threads.insert(String::from(key), read_thread);
		}
		return self.read_threads.get(key).unwrap();
	}

	fn init_read_threads(&mut self) {
		for key in self.config.uniq_in() {
			self.init_read_thread(&key);
		}
	}
	*/

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
		let read = read_threads.by_name(&line.r#in);
		let out = output_devices.by_name(&line.out);
		/*
		let read = match self.read_threads.get(&line.r#in) {
			Some(read) => read,
			None => read_threads.by_name(&line.r#in)
		};
		*/
		/*
		let out = match output_devices.get(&line.out) {
			Some(out) => out,
			None => { return; }, //App::init_output_thread(&line.out) },
		};
		*/
		read.setup_write( out.clone(), line.func.to_string(), line.args.to_string());
	}

	pub fn ready(&mut self) {
		self.flags.run.store(true, Ordering::Relaxed);
		for (_name,thread) in self.read_threads.0.iter_mut() {
			thread.flags.run.store(true, Ordering::Relaxed);
		}
	}

	pub fn join(&mut self) {
		println!("joining");
		loop {
			if self.flags.hup.swap(false, Ordering::Relaxed) {
				println!("Got SIGHUP");
				for (_name,thread) in self.read_threads.0.iter_mut() {
					thread.flags.hup.store(true, Ordering::Relaxed);
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
