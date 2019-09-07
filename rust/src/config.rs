#[derive(Debug)]

pub struct ConfigLine {
	r#in: String,
	out: String,
	func: String,
	args: String
}

pub fn load_configuration() -> Vec<ConfigLine> {
	csv::ReaderBuilder::new()
		.delimiter(b'\t')
		.from_path("/home/pi/Development/midibus/server/midi-server.conf")
		.unwrap()
		.records()
		.map(|rec| {
			let r = rec.unwrap();
			ConfigLine {
				r#in: r[0].to_string(),
				out: r[1].to_string(),
				func: r[2].to_string(),
				args: r[3].to_string()
			}
		}).collect::<Vec<ConfigLine>>()
}

