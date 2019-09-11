extern crate alsa;

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
	pub midi : Option<Arc<Mutex<Rawmidi>>>,
	port_name : String,
	outs : Vec<WriteData>,
	macros : Vec<MacroListener>,
	hup : Arc<AtomicBool>,
}
impl fmt::Debug for ReadData {
	fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
		write!(f, "ReadData {{ port_name: {}, outs: {:#?}, macros: {:#?} }}", self.port_name, self.outs, self.macros)
	}
}
impl ReadData {
	pub fn new( name : String, hup : Arc<AtomicBool> ) -> ReadData {
		ReadData {
			midi: None,
			port_name: name,
			outs: vec![],
			macros: vec![],
			hup : hup.clone(),
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
	pub fn scan_midi(&mut self) {
		match Rawmidi::new(&self.port_name, Direction::input(), false) {
			Ok(midi) => {
				println!("Opened {}", self.port_name);
				let arc = Arc::new(Mutex::new(midi));
				for w in self.outs.iter_mut() {
					w.update_midi_in(arc.clone());
				}
				self.midi = Some(arc.clone())
			},
			Err(e) => {
				self.midi = None
			}
		}
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
	pub fn new(port_name: String ) -> ReadThread {
		let hup = Arc::new(AtomicBool::new(false));
		let data = RwLock::new(ReadData::new(port_name, hup.clone()));
		let arc = Arc::new(data);
		ReadThread {
			hup: hup.clone(),
			data: arc.clone(),
			handle: thread::spawn(move || {
				'outer: loop {
					{
						arc.write().unwrap().scan_midi();
					}
					{
						match &arc.read().unwrap().midi {
							Some(midi) => {
								'read: loop {
									let m = midi.as_ref().lock().unwrap();
									let mut buf : Vec<u8> = vec![0; BUFSIZE];
									match m.io().read(&mut buf) {
										Ok(n) => {
											arc.write().unwrap().handle_read(buf[0..n].to_vec());
										},
										Err(_) => {
											println!("Error reading from {}", arc.read().unwrap().port_name);
											break;
										},
									}
								}
							},
							None => {},
						}
					}
					let mut tries = 0;
					loop {
						tries += 1;
						thread::sleep(Duration::from_millis(1000));
						if tries % 5 == 0 {
							println!("Snoozing");
							let mut snooze = 10;
							while snooze > 0 {
								if hup.swap(false, Ordering::Relaxed) {
									println!("I'm awake!");
									break;
								}
								thread::sleep(Duration::from_millis(1000));
								snooze -= 1;
							}
						}
					}
				}
			}),
		}
	}
	pub fn join(self) {
		self.handle.join().unwrap();
	}
	pub fn setup_write(&mut self, out : Arc<OutputDevice>, func : String, args : String) {
		let mut data = self.data.write().unwrap();
		data.setup_write(out,func,args);
	}
}
