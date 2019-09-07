extern crate alsa;

use alsa::rawmidi::Rawmidi;
use crate::r#macro::MacroListener;

pub struct OutputDevice {
    midi : Option<Rawmidi>,
    status : u8,
    midi_in_exclusive : Option<Rawmidi>,
    port_name : String
}

pub struct ReadThreadData {
    midi : Option<Rawmidi>,
    //outs : Vec<WriteData>,
    port_name : String,
    macros : Vec<MacroListener>
}
