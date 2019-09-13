
#[derive(Debug)]
pub struct CCMap { channel : u8, out_cc : Vec<u8> }
impl CCMap {
	pub fn new( args : String ) -> CCMap {
		let args : Vec<&str> = args.split(",").collect();
		CCMap { channel : parse_hex(args[0]), out_cc : vec![] }
	}
}
impl CallbackFn for CCMap {
	fn callback(&self, _data : &mut CallbackData, _buf : &Vec<u8>) -> usize {
		//println!("{}", buf);
		0
	}
}
