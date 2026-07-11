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

#[cfg(test)]
mod tests {
    use super::*;
    use std::fs;

    #[test]
    fn test_fileio_read_write() {
        let fm = FileManager::new();
        let filepath = "test_io_file.txt";
        let content = b"hello test file";

        // Test Write
        assert!(fm.write_file(filepath, content).is_ok());

        // Test Read
        let read_content = fm.read_file(filepath).unwrap();
        assert_eq!(read_content, content);

        // Cleanup
        fs::remove_file(filepath).unwrap();
    }
}
