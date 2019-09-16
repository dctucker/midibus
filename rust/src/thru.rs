extern crate libc;
extern crate alsa;

use std::sync::atomic::Ordering;
use std::io::Read;
use alsa::rawmidi::Rawmidi;
use alsa::Direction;
//use alsa::card::Iter as CardIter;
//use alsa::rawmidi::Iter;
//use alsa::ctl::Ctl;
use std::sync::RwLock;
use std::sync::Arc;
use std::fmt;
use std::thread;
//use std::thread::JoinHandle;
use std::time::Duration;

use crate::app::Flags;
use crate::r#macro::MacroListener;
use crate::output::OutputDevice;
use crate::write::WriteData;

pub const BUFSIZE : usize = 1024;

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
	pub fn setup_write(&mut self, out : &Arc<RwLock<OutputDevice>>, func : String, args : String) {
		let port_name = out.read().unwrap().port_name.clone();
		for wd in self.outs.iter_mut() {
			if wd.func_name == func && wd.output_device.read().unwrap().port_name == port_name {
				wd.add_args(args);
				return;
			}
		}
		self.outs.push( WriteData::new(&out, func, args) );

	}
	fn handle_read(&mut self, buf : &[u8]) {
		print!("I");
		for x in buf.iter(){ print!(" 0x{:02x}", x); }
		println!(" {}", self.port_name);
		for w in self.outs.iter_mut() {
			w.call(buf);
		}
	}
}

pub struct ReadThread {
	pub handle : std::thread::JoinHandle<()>,
	data : Arc<RwLock<ReadData>>,
	pub flags : Arc<Flags>,
}
impl fmt::Debug for ReadThread {
	fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
		write!(f, "ReadThread {{ data: {:#?} }}", self.data)
	}
}
impl ReadThread {
	fn read_loop( flags : &Arc<Flags>, arc : &Arc<RwLock<ReadData>>, midi : &Rawmidi ) {
		/*
		// calculate BPM
		let mut clock = std::time::Instant::now();
		let mut clock1;
		const BPM_SIZE : usize = 32;
		let mut bpms = vec![ 0.0f64 ; BPM_SIZE];
		let mut bpm_offset : usize = 0;
		*/
		let mut buf = [0u8; BUFSIZE];
		'read: loop {
			use alsa::PollDescriptors;
			while ! flags.run.load(Ordering::Relaxed) {
				if flags.hup.swap(false, Ordering::Relaxed) { break 'read; }
				thread::sleep(Duration::from_millis(500));
			}
			//thread::yield_now();
			if flags.int.load(Ordering::Relaxed) { break 'read; }

			let res = alsa::poll::poll(&mut midi.get().unwrap(), 500).unwrap();
			//clock1 = std::time::Instant::now();
			if res == 0 { continue; }

			match midi.io().read(&mut buf) {
				Ok(n) => {
					/*
					// calculate BPM
					for c in &buf[0..n] {
						if *c == 0xf8 {
							let bpm = 2_500_000.0f64 / (clock1.duration_since(clock).as_micros() as f64);
							bpms[ bpm_offset ] = bpm;
							bpm_offset = (bpm_offset + 1) % BPM_SIZE;
							let mut sum : f64 = 0.0;
							for b in &bpms {
								sum += b;
							}
							sum /= BPM_SIZE as f64;
							println!("Tempo = {:?} BPM", sum);
							clock = clock1;
							break;
						}
					}
					*/
					arc.write().unwrap().handle_read( &buf[0..n] );
				},
				Err(_) => {
					println!("Error reading");
					break 'read;
				},
			}
		}
	}

	fn wait_loop( flags : &Arc<Flags> ) {
		let mut tries = 0;
		'wait: loop {
			tries += 1;
			thread::sleep(Duration::from_millis(500));
			if flags.hup.swap(false, Ordering::Relaxed) { break 'wait; }
			if flags.int.load(Ordering::Relaxed) { break 'wait; }
			if tries % 5 == 0 {
				let mut snooze = 10;
				while snooze > 0 {
					if flags.hup.swap(false, Ordering::Relaxed) { break 'wait; }
					if flags.int.load(Ordering::Relaxed) { break 'wait; }
					thread::sleep(Duration::from_millis(1000));
					snooze -= 1;
				}
			}
		}
	}
	pub fn new(port_name: String ) -> ReadThread {
		let flags = Arc::new( Flags::new() );
		let flags2 = flags.clone();
		let data = RwLock::new(ReadData::new(port_name));
		let arc = Arc::new(data);
		let arc2 = arc.clone();
		let routine = move || {
			let port_name = arc.read().unwrap().port_name.clone();

			while ! &flags.run.load(Ordering::Relaxed) {
				thread::sleep(Duration::from_millis(1000));
			}
			'outer: loop {
				if flags.int.load(Ordering::Relaxed) { break 'outer; }
				println!("Scanning for {}", port_name);
				match Rawmidi::new(&port_name, Direction::input(), false) {
					Err(_) => {},
					Ok(midi) => {
						println!("Opened {} for input", port_name);
						ReadThread::read_loop(&flags, &arc, &midi);
					},
				};
				thread::yield_now();
				ReadThread::wait_loop(&flags);
			}
			println!("Read thread {} done", port_name);
			flags.int.store(false, Ordering::Relaxed);
		};
		let handle = thread::spawn(routine);
		println!("Spawning new thread");

		ReadThread {
			flags: flags2,
			data: arc2,
			handle: handle,
		}
	}
	/*
	pub fn join(&self) {
		//self.handle.join();
	}
	*/
	pub fn setup_write(&self, out : &Arc<RwLock<OutputDevice>>, func : String, args : String) {
		self.data.write().unwrap().setup_write(out,func,args);
	}
}
