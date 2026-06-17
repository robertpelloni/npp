use std::fs;
use std::io;
use std::path::Path;

pub struct FileManager;

impl FileManager {
    pub fn new() -> Self {
        Self
    }

    pub fn read_file<P: AsRef<Path>>(&self, filepath: P) -> io::Result<Vec<u8>> {
        fs::read(filepath)
    }

    pub fn write_file<P: AsRef<Path>>(&self, filepath: P, content: &[u8]) -> io::Result<()> {
        fs::write(filepath, content)
    }
}
