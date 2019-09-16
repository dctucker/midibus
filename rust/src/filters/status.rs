use std::convert::TryInto;

pub struct Status { out_status : [u8 ; 0x80] }
impl Status {
	pub fn new( args : String ) -> Status {
		let mut out_status = [ 0 ; 0x80 ];
		for arg in args.split(",") {
			let status = parse_hex(arg);
			let i : usize = ((0x80 | status) - 0x80).try_into().unwrap();
			out_status[i] = status;
		}
		Status { out_status : out_status }
	}
}
impl CallbackFn for Status {
	fn callback(&self, data : &mut CallbackData, buf : &[u8]) -> usize {
		let mut a = 0;
		let out_status = &self.out_status;

		for c in buf {
			let cur_state = data.output_device.scan_status(*c);
			let i : usize = ((0x80 | cur_state) - 0x80).try_into().unwrap();
			if out_status[i] == cur_state {
				data.buffer[ a ] = *c;
				a += 1;
			}
		}
		data.output_device.send_buffer(&data.buffer[0..a]).unwrap()
	}
}
