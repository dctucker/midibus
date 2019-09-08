use std::collections::{HashMap,HashSet};

#[derive(Debug)]
pub struct ConfigLine {
	pub r#in: String,
	pub out: String,
	pub func: String,
	pub args: String
}

#[derive(Debug)]
pub struct Config( Vec<ConfigLine> );

impl Config {
	pub fn from_path(path: &str) -> Config {
		Config( csv::ReaderBuilder::new()
			.delimiter(b'\t')
			.from_path(path)
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
		)
	}
	pub fn new() -> Config { Config::from_path("/home/pi/Development/midibus/server/midi-server.conf") }
	pub fn lines(&self) -> &Vec<ConfigLine> { &self.0 }
	pub fn uniq_in(&self) -> HashSet<String> {
		self.lines().into_iter()
			.map( |line| line.r#in.clone() )
			.collect::<HashSet<String>>()
	}
	pub fn uniq_out(&self) -> HashSet<String> {
		self.lines().into_iter()
			.map( |line| line.out.clone() )
			.collect::<HashSet<String>>()
	}
}
