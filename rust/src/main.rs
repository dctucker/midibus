extern crate signal_hook;
extern crate alsa;
extern crate csv;

use std::io::Error;
use std::sync::Arc;

mod app;
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
	signal_hook::flag::register(signal_hook::SIGHUP, Arc::clone(&app.hup))?;

	println!("{:#?}", app);

	app.join();
	Ok(())
}
