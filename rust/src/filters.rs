use enum_dispatch::enum_dispatch;
use crate::output::OutputDevice;

const MASK_SYSEX : u32 = 1 << 20;
const MASK_RT    : u32 = 1 << 21;
const MASK_ALL   : u32 = 0x1fffe;

#[derive(Debug)]
pub struct Void { }
impl Void { pub fn new( _args : String ) -> Void { Void { } } }
impl CallbackFn for Void { fn callback(&self, _write_data : &mut OutputDevice, _buf : &Vec<u8>) -> usize { 0 } }

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
	fn callback(&self, write_data : &mut OutputDevice, buf : &Vec<u8>) -> usize {
		write_data.send_buffer(buf).unwrap();
		//println!("{}", buf);
		0
	}
}

#[derive(Debug)]
pub struct Funnel { channel : u8 }
impl Funnel {
	pub fn new( args : String ) -> Funnel {
		Funnel { channel : args.parse().unwrap() }
	}
}
impl CallbackFn for Funnel {
	fn callback(&self, _write_data : &mut OutputDevice, _buf : &Vec<u8>) -> usize {
		//println!("{}", buf);
		0
	}
}

#[derive(Debug)]
pub struct CCMap { channel : u8, out_cc : Vec<u8> }
impl CCMap {
	pub fn new( args : String ) -> CCMap {
		CCMap { channel : args.parse().unwrap(), out_cc : vec![] }
	}
}
impl CallbackFn for CCMap {
	fn callback(&self, _write_data : &mut OutputDevice, _buf : &Vec<u8>) -> usize {
		//println!("{}", buf);
		0
	}
}

#[derive(Debug)]
pub struct Status { out_status : Vec<u8> }
impl Status {
	pub fn new( _args : String ) -> Status {
		Status { out_status : vec![] }
	}
}
impl CallbackFn for Status {
	fn callback(&self, _write_data : &mut OutputDevice, _buf : &Vec<u8>) -> usize {
		//println!("{}", buf);
		0
	}
}

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
	fn callback(&self, _write_data : &mut OutputDevice, _buf : &Vec<u8>) -> usize;
}

impl Callback {
	pub fn new( name : String, args : String ) -> Callback {
		match name.as_ref() {
			"channel" => Channel::new(args).into(),
			"funnel"  =>  Funnel::new(args).into(),
			"ccmap"   =>   CCMap::new(args).into(),
			"status"  =>  Status::new(args).into(),
			_ => Void::new(args).into(),
		}
	}
}
