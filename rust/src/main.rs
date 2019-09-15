extern crate signal_hook;
extern crate alsa;
extern crate csv;

use std::io::Error;
use std::sync::Arc;

mod app;
mod devices;
mod config;
mod thru;
mod r#macro;
mod write;
mod filters;
mod output;
mod lib;

fn main() -> Result<(), Error> {
	println!("Starting up");
	let mut app = app::App::new();
	signal_hook::flag::register(signal_hook::SIGHUP, Arc::clone(&app.flags.hup))?;
	signal_hook::flag::register(signal_hook::SIGINT, Arc::clone(&app.flags.int))?;

	// println!("{:#?}", app);

	app.join();
	Ok(())
}
