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
			//let cur_state = scan_status(data, buf[b]);
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

#[derive(Debug)]
pub struct Status { out_status : Vec<u8> }
impl Status {
	pub fn new( _args : String ) -> Status {
		Status { out_status : vec![] }
	}
}
impl CallbackFn for Status {
	fn callback(&self, _data : &mut CallbackData, _buf : &Vec<u8>) -> usize {
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
	fn callback(&self, _data : &mut CallbackData, _buf : &Vec<u8>) -> usize;
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
