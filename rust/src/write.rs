
enum WriteArgs {
	struct ChannelFilterData {
		mask : u32,
	}
	struct FunnelFilterData {
		channel : u8,
	}
	struct CCMapFilterData {
		channel : u8,
		out_cc : Vec<u8>,
	}
	struct CCMapStatusData {
		out_status : Vec<u8>,
	}
}

trait WriteCallbackFn {
	fn callback(write_data : &WriteData, args &WriteArgs, &buf : Vec<u8>, &out_buf : Vec<u8>) -> usize;
}

struct WriteCallback {
	WriteArgs args;
	out_buf Vec<u8>;
}

impl WriteCallbackFn for WriteCallback {
	fn callback(write_data : &WriteData, &buf : Vec<u8>) -> usize {
	}
}

struct WriteData {
	output_device : OutputDevice,
	midi_in : Rawmidi,
	port_name : String,
	func_name : String,
	args_name : String,
	callbacks : Vec<WriteCallback>,
}
