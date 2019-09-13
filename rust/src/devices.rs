use std::sync::{Arc,RwLock};
use crate::thru::ReadThread;
use crate::output::OutputDevice;
use std::collections::HashMap;

pub trait Create<T> {
	fn create( key : &String ) -> T;
}

#[derive(Debug)]
pub struct DeviceMap<T> {
	map : HashMap<String,T>
}
impl<T> DeviceMap<T> {
	pub fn new() -> DeviceMap<T> { DeviceMap { map: HashMap::new() } }
}
impl<T> std::ops::Deref for DeviceMap<T> {
	type Target = HashMap<String,T>;
	fn deref(&self) -> &Self::Target { &self.map }
}
impl<T> std::ops::DerefMut for DeviceMap<T> {
	fn deref_mut(&mut self) -> &mut Self::Target { &mut self.map }
}
impl<T> DeviceMap<T> where DeviceMap<T> : Create<T> {
	pub fn by_name( &mut self, key : &String ) -> &T {
		if ! self.contains_key(key) {
			self.insert(String::from(key), Self::create(key));
		}
		self.get(key).unwrap()
	}
}

pub type ReadThreadMap = DeviceMap<ReadThread>;
impl Create<ReadThread> for ReadThreadMap {
	fn create( key : &String ) -> ReadThread {
		println!("New input device {}", key);
		ReadThread::new(String::from(key))
	}
}

pub type OutputDeviceMap = DeviceMap<Arc<RwLock<OutputDevice>>>;
impl Create<Arc<RwLock<OutputDevice>>> for OutputDeviceMap {
	fn create( key : &String ) -> Arc<RwLock<OutputDevice>> {
		Arc::new( RwLock::new( OutputDevice::new(key.clone() ) ) )
	}
}

