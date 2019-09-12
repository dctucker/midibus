extern crate alsa;

use std::fmt;
use std::io::Write;
use std::sync::Mutex;
use alsa::rawmidi::Rawmidi;

pub struct OutputDevice {
	midi : Option<Mutex<Rawmidi>>,
	status : u8,
	//midi_in_exclusive : SafeRawmidi,
	pub port_name : String
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
			//midi_in_exclusive: None,
		}
	}
	pub fn send_buffer(&mut self, buf : &Vec<u8> ) -> Result<usize, String> {
		let midi = match &self.midi {
			Some(mutex) => match mutex.lock() {
				Ok(m) => m,
				Err(_) => return Err(format!("Could not lock {}", self.port_name)),
			},
			None => {
				println!("Device not connected {}", self.port_name);
				return Ok(0)
			},
		};
		match midi.io().write(buf) {
			Ok(n) => {
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
}
