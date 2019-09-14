use enum_dispatch::enum_dispatch;
//use crate::output::OutputDevice;
use crate::write::CallbackData;
use crate::thru::BUFSIZE;

const MASK_SYSEX : u32 = 1 << 20;
const MASK_RT    : u32 = 1 << 21;
const MASK_ALL   : u32 = 0x1fffe;

fn parse_hex(arg : &str) -> u8 {
	if arg.starts_with("0x") {
		let without_prefix = arg.trim_start_matches("0x");
		u8::from_str_radix(without_prefix, 16).unwrap()
	} else {
		arg.parse::<u8>().unwrap()
	}
}

#[derive(Debug)]
pub struct Void { }
impl Void { pub fn new( _args : String ) -> Void { Void { } } }
impl CallbackFn for Void { fn callback(&self, _data : &mut CallbackData, _buf : &Vec<u8>) -> usize { 0 } }

include!("channel.rs");
include!("funnel.rs");
include!("ccmap.rs");
include!("status.rs");

#[enum_dispatch]
#[derive(Debug)]
pub enum Callback {
	Void,
	Channel,
	Funnel,
	CCMap,
	Status,
}

#[enum_dispatch(Callback)]
pub trait CallbackFn {
	fn callback(&self, _data : &mut CallbackData, _buf : &Vec<u8>) -> usize;
	fn add_args( &mut self, args : String ) {}
}

impl Callback {
	pub fn new( name : String, args : String ) -> Callback {
		match name.as_ref() {
			"channel" => Channel::new(args).into(),
			"funnel"  =>  Funnel::new(args).into(),
			"ccmap"   =>   CCMap::new(args).into(),
			"status"  =>  Status::new(args).into(),
			_         =>    Void::new(args).into(),
		}
	}
}
