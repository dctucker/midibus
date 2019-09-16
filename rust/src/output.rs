extern crate alsa;

use std::fmt;
use std::io::Write;
use std::sync::Mutex;
use alsa::rawmidi::Rawmidi;

pub struct OutputDevice {
	midi : Option<Mutex<Rawmidi>>,
	status : u8,
	pub midi_in_exclusive : String,
	pub port_name : String
}

impl fmt::Debug for OutputDevice {
	fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
		write!(f, "OutputDevice {{ port_name: {} }}", self.port_name)
	}
}

impl OutputDevice {
	pub fn new(name : String) -> OutputDevice {
		let mut out = OutputDevice {
			midi: None,
			port_name: name,
			status: 0,
			midi_in_exclusive: "".to_string(),
		};
		out.connect();
		out
	}
	pub fn connect( &mut self ) {
		match self.midi {
			Some(_) => { return; },
			None => {},
		};
		self.midi = match Rawmidi::new(&self.port_name, alsa::Direction::output(), false) {
			Ok(midi) => {
				println!("Opened {} for output", self.port_name);
				Some( Mutex::new( midi ) )
			},
			Err(_) => {
				println!("Could not open {} for output", self.port_name);
				None
			},
		}
	}
	pub fn send_buffer(&mut self, buf : &[u8] ) -> Result<usize, String> {
		if buf.len() == 0 {
			return Ok(0)
		}
		let midi = match &self.midi {
			Some(mutex) => match mutex.lock() {
				Ok(m) => m,
				Err(_) => return Err(format!("Could not lock {}", self.port_name)),
			},
			None => {
				//println!("Device not connected {}", self.port_name);
				return Ok(0)
			},
		};
		match midi.io().write(buf) {
			Ok(n) => {
				self.handle_write(buf);
				let mut status : u8 = self.status;
				for c in buf.iter() {
					if *c >= 0x80 {
						status = *c
					}
				}
				self.status = status;
				Ok(n)
			},
			Err(_) => Err(format!("Error writing to {}", self.port_name)),
		}
	}
	fn handle_write(&self, buf : &[u8]) {
		print!("O");
		for x in buf.iter(){ print!(" 0x{:02x}", x); }
		println!(" {}", self.port_name);
	}
	pub fn scan_status( &mut self, status : u8 ) -> u8 {
		if status < 0x80 {
			return self.status;
		}
		self.status = status;
		self.status
	}
}
