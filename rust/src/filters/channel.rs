
#[derive(Debug)]
pub struct Channel { mask : u32 }
impl Channel {
	pub fn new( args : String ) -> Channel {
		let mut mask : u32 = 0;
		//println!("{}", args);
		for arg in args.split(",") {
			mask |= match arg {
				"sysex" => MASK_SYSEX,
				"all" => MASK_ALL,
				"rt" => MASK_RT,
				_ => 1 << parse_hex(arg),
			};
		}
		Channel { mask: mask }
	}
}
impl CallbackFn for Channel {
	fn callback(&self, data : &mut CallbackData, buf : &Vec<u8>) -> usize {
		let mask = self.mask;
		let mut out_buf = [0u8 ; BUFSIZE];
		let mut current_mask = 0;

		let mut output_device = data.output_device.write().unwrap();
		if output_device.midi_in_exclusive == data.midi_in {
			current_mask = MASK_SYSEX;
		}

		let mut a = 0;
		for c in buf {
			let _cur_state = output_device.scan_status(*c);
			if *c >= 0xf8 {
				current_mask = MASK_RT;
			} else if *c >= 0xf0 {
				if *c == 0xf0 {
					output_device.midi_in_exclusive = data.midi_in.clone();
				} else if *c == 0xf7 {
					output_device.midi_in_exclusive = "".to_string();
				}
				current_mask = MASK_SYSEX;
			} else if *c >= 0x80 {
				current_mask = 2 << (c & 0x0f);
			}

			if mask & current_mask != 0 {
				out_buf[a] = *c;
				a += 1;
			}
		}
		output_device.send_buffer(&out_buf[0..a].to_vec()).unwrap()
	}
}
