extern crate alsa;
extern crate csv;

mod app;
mod config;
mod thru;
mod r#macro;
mod devices;

fn main() {
	let mut app = app::App::new();
	//let read_threads = thru::
	println!("{:?}", app.config_lines);
	app.add_macro();

	devices::main();

	if false { devices::main() }
}