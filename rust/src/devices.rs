use std::io::Read;

use alsa::rawmidi::Iter as RawMidiIter;
use alsa::Direction;
use alsa::rawmidi::Rawmidi;
use alsa::card::Iter as CardIter;
use alsa::ctl::Ctl;

fn list_cards() {
	let cards = CardIter::new();
	for (_i,c) in cards.enumerate() {
		let card = c.unwrap();
		println!("{} \"{}\":", card.get_longname().unwrap(), card.get_name().unwrap());
		let ctl = Ctl::from_card(&card, true).unwrap();
		let raws = RawMidiIter::new(&ctl);
		for (_i,r) in raws.enumerate() {
			let raw = r.unwrap();
			println!("\t{}: {} ({:?})",
				raw.get_id().unwrap(),
				raw.get_subdevice_name().unwrap(),
				raw.get_stream()
			);
		}
	}
}

pub fn main() {
	/*
	use std::ffi::CString;
	use alsa::device_name::HintIter;

	for t in &["rawmidi"] {
		println!("{} devices:", t);
		let i = HintIter::new(None, &*CString::new(*t).unwrap()).unwrap();
		for a in i { println!("  {:?}", a) }
	}
	list_cards();
	*/
	let mio = Rawmidi::new("hw:mio", Direction::input(), false).unwrap();
	for data in mio.io().bytes() {
		println!("{}", data.unwrap());
	}
}
