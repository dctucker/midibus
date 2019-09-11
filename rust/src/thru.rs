extern crate libc;
extern crate alsa;

use timeout_readwrite::TimeoutReader;
use std::sync::atomic::{AtomicBool, Ordering};
use std::io::Read;
use alsa::rawmidi::Rawmidi;
use alsa::card::Iter as CardIter;
use alsa::rawmidi::Iter;
use alsa::Direction;
use alsa::ctl::Ctl;
use std::sync::{Mutex,RwLock};
use std::sync::Arc;
use std::fmt;
use std::thread;
use std::thread::JoinHandle;
use std::time::Duration;
use crate::r#macro::MacroListener;
use crate::output::OutputDevice;
use crate::write::WriteData;
use crate::lib::SafeRawmidi;

const BUFSIZE : usize = 1024;

pub struct ReadData {
	port_name : String,
	outs : Vec<WriteData>,
	macros : Vec<MacroListener>,
}
impl fmt::Debug for ReadData {
	fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
		write!(f, "ReadData {{ port_name: {}, outs: {:#?}, macros: {:#?} }}", self.port_name, self.outs, self.macros)
	}
}
impl ReadData {
	pub fn new( name : String ) -> ReadData {
		ReadData {
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
	fn handle_read(&self, buf : Vec<u8>) {
		print!("I");
		for x in buf.iter(){ print!(" 0x{:02x}", x); }
		println!(" {}", self.port_name);
		for w in self.outs.iter() {
			w.call(&buf);
		}
	}

	/*
	fn scan_loop() {
		'scan: loop {
			for a in CardIter::new().map(|a| a.unwrap()) {
				for b in Iter::new(&Ctl::from_card(&a, false).unwrap()).map(|b| b.unwrap()) {
					if b.get_stream() == Direction::Capture {
						if b.get_subdevice_name().unwrap() == self.port_name {
							break 'scan;
						}
					}
					println!("{:?} {}", b.get_stream(), b.get_id().unwrap());
				}
			}
			tries += 1;
			thread::sleep(Duration::from_millis(1000));
			if tries % 5 == 0 {
				thread::sleep(Duration::from_millis(10000));
			}
		}
	}
	*/
}

pub struct ReadThread {
	handle : JoinHandle<()>,
	data : Arc<RwLock<ReadData>>,
	pub hup : Arc<AtomicBool>,
}
impl fmt::Debug for ReadThread {
	fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
		write!(f, "ReadThread {{ data: {:#?} }}", self.data)
	}
}
impl ReadThread {
	fn read_loop( arc : &Arc<RwLock<ReadData>>, midi : &Rawmidi ) {
		'read: loop {
			use alsa::PollDescriptors;
			let mut buf : Vec<u8> = vec![0; BUFSIZE];
			//println!("reading from {}", self.port_name);
			thread::yield_now();
			let res = alsa::poll::poll(&mut midi.get().unwrap(), 500).unwrap();
			if res == 0 { continue; }
			match midi.io().read(&mut buf) {
				Ok(n) => {
					arc.write().unwrap().handle_read( buf.to_vec() );
				},
				Err(_) => {
					println!("Error reading");
					break 'read;
				},
			}
		}
	}

	fn wait_loop( hup : &Arc<AtomicBool> ) {
		let mut tries = 0;
		'wait: loop {
			tries += 1;
			thread::sleep(Duration::from_millis(500));
			if hup.swap(false, Ordering::Relaxed) { break 'wait; }
			if tries % 5 == 0 {
				let mut snooze = 10;
				while snooze > 0 {
					if hup.swap(false, Ordering::Relaxed) { break 'wait; }
					thread::sleep(Duration::from_millis(1000));
					snooze -= 1;
				}
			}
		}
	}
	pub fn new(port_name: String ) -> ReadThread {
		let hup = Arc::new(AtomicBool::new(false));
		let hup2 = hup.clone();
		let data = RwLock::new(ReadData::new(port_name));
		let arc = Arc::new(data);
		let arc2 = arc.clone();
		let routine = move || {
			let port_name = arc.read().unwrap().port_name.clone();
			println!("Starting outer loop {}", port_name);
			'outer: loop {
				println!("Scanning for {}", port_name);
				match Rawmidi::new(&port_name, Direction::input(), false) {
					Err(_) => {},
					Ok(midi) => {
						println!("Opened {}", port_name);
						ReadThread::read_loop(&arc, &midi);
					},
				};
				thread::yield_now();
				ReadThread::wait_loop(&hup);
			}
		};
		let handle = thread::spawn(routine);
		println!("Spawning new thread");

		ReadThread {
			hup: hup2,
			data: arc2,
			handle: handle,
		}
	}
	pub fn join(self) {
		self.handle.join().unwrap();
	}
	pub fn setup_write(&self, out : Arc<OutputDevice>, func : String, args : String) {
		self.data.write().unwrap().setup_write(out,func,args);
	}
}
