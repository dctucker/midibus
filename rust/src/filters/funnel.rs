
#[derive(Debug)]
pub struct Funnel { channel : u8 }
impl Funnel {
	pub fn new( args : String ) -> Funnel {
		Funnel { channel : parse_hex(&args) }
	}
}
impl CallbackFn for Funnel {
	fn callback(&self, _data : &mut CallbackData, _buf : &Vec<u8>) -> usize {
		//println!("{}", buf);
		0
	}
}
