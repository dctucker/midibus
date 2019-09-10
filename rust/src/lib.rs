use std::sync::{Mutex,RwLock};
use alsa::rawmidi::Rawmidi;

pub type SafeRawmidi = Option<Mutex<alsa::rawmidi::Rawmidi>>;
