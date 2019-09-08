use enum_dispatch::enum_dispatch;
use crate::write::WriteData;

const MASK_SYSEX : u32 = 1 << 20;
const MASK_RT    : u32 = 1 << 21;
const MASK_ALL   : u32 = 0x1fffe;

#[derive(Debug)]
struct Channel {
	mask : u32,
}
impl Channel {
	pub fn new( args : String ) -> Channel {
		let mut mask : u32 = 0;
		println!("{}", args);
		for arg in args.split(",") {
			mask |= match arg {
				"sysex" => MASK_SYSEX,
				"all" => MASK_ALL,
				"rt" => MASK_RT,
				_ => {
					if arg.starts_with("0x") {
						let without_prefix = arg.trim_start_matches("0x");
						1 << u32::from_str_radix(without_prefix, 16).unwrap()
					} else {
						1 << arg.parse::<u8>().unwrap()
					}
				},
			};
		}
		Channel { mask: mask }
	}
}
impl CallbackFn for Channel {
	fn callback(&mut self, write_data : WriteData, buf : &Vec<u8>) -> usize {
		//println!("{}", buf);
		0
	}
}

#[derive(Debug)]
struct Void {
}
impl Void {
	pub fn new( args : String ) -> Void { Void { } }
}
impl CallbackFn for Void {
	fn callback(&mut self, write_data : WriteData, buf : &Vec<u8>) -> usize {
		//println!("{}", buf);
		0
	}
}

#[enum_dispatch]
#[derive(Debug)]
pub enum Callback {
	Channel,
	Void,
	/*
	Funnel {
		channel : u8,
	},
	CCMap {
		channel : u8,
		out_cc : Vec<u8>,
	},
	Status {
		out_status : Vec<u8>,
	},
	*/
}

#[enum_dispatch(Callback)]
pub trait CallbackFn {
	fn callback(&mut self, write_data : WriteData, buf : &Vec<u8>) -> usize;
}

impl Callback {
	pub fn new( name : String, args : String ) -> Callback {
		match name.as_ref() {
			"channel" => Channel::new(args).into(),
			_ => Void::new(args).into(),
		}
	}
}
