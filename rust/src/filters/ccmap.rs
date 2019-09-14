
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
		//println!("{}", buf);
		0
	}
}
