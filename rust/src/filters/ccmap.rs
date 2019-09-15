
#[derive(Debug)]
pub struct CCMap { channel : u8, out_cc : Vec<u8> }
impl CCMap {
	pub fn new( args : String ) -> CCMap {
		let out_cc = vec![ 0 ; 0xff ];
		let channel = args.split(",").next().unwrap();

		let mut ccmap = CCMap {
			channel : parse_hex(channel),
			out_cc : out_cc,
		};
		ccmap.add_args( args );
		ccmap
	}
}
impl CallbackFn for CCMap {
	fn add_args( &mut self, args : String ) {
		let mut arg = args.split(",");
		self.channel = parse_hex(arg.next().unwrap()) - 1;
		let in_cc : usize = parse_hex(arg.next().unwrap()).try_into().unwrap();
		self.out_cc[in_cc] = parse_hex( arg.next().unwrap() );
		println!("ccmap {} CC {} to {}",
			self.channel,
			in_cc,
			self.out_cc[in_cc]
		);
	}
	fn callback(&self, data : &mut CallbackData, buf : &Vec<u8>) -> usize {
		let exp_state : u8 = 0xb0 + self.channel;
		let mut out_buf = [0u8 ; BUFSIZE];
		let mut cc : u8 = 0xff;
		let mut val : u8 = 0xff;

		let mut a = 0;
		for c in buf {
			let cur_state : u8 = data.output_device.scan_status(*c);
			if cur_state == exp_state && *c < 0x80 {
				if cc == 0xff {
					let s : usize = (*c).try_into().unwrap();
					let out_cc : u8 = self.out_cc[s];
					if out_cc < 0x80 {
						cc = out_cc;
					} else {
						cc = 0xfe;
					}
				} else if val == 0xff {
					if cc < 0x80 {
						val = *c;
						out_buf[a] = exp_state; a += 1;
						out_buf[a] = cc; a += 1;
						out_buf[a] = val; a += 1;
					}
					val = 0xff;
					cc = 0xff;
				}
			} else {
				cc = 0xff;
				val = 0xff;
			}
		}
		data.output_device.send_buffer(&out_buf[0..a].to_vec()).unwrap()
	}
}
