
#[derive(Debug)]
pub struct Funnel { channel : u8 }
impl Funnel {
	pub fn new( args : String ) -> Funnel {
		Funnel { channel : parse_hex(&args) }
	}
}
impl CallbackFn for Funnel {
	fn callback(&self, data : &mut CallbackData, buf : &Vec<u8>) -> usize {
		let mask = MASK_ALL;
		let mut out_buf = [0u8 ; BUFSIZE];
		let mut current_mask = 0;

		if data.output_device.midi_in_exclusive == data.midi_in {
			current_mask = MASK_SYSEX;
		}

		let mut a = 0;
		for c in buf {
			let _cur_state = data.output_device.scan_status(*c);
			if *c >= 0xf8 {
				current_mask = MASK_RT;
			} else if *c >= 0xf0 {
				if *c == 0xf0 {
					data.output_device.midi_in_exclusive = data.midi_in.clone();
				} else if *c == 0xf7 {
					data.output_device.midi_in_exclusive = "".to_string();
				}
				current_mask = MASK_SYSEX;
			} else if *c >= 0x80 {
				current_mask = 2 << (*c & 0x0f);
				out_buf[a] = *c & 0xf0 | self.channel;
				a += 1;
				continue;
			}

			if mask & current_mask != 0 {
				out_buf[a] = *c;
				a += 1;
			}
		}
		data.output_device.send_buffer(&out_buf[0..a].to_vec()).unwrap()
	}
}
