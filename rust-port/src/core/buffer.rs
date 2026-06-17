use crate::core::eventbus::EventBus;
use parking_lot::RwLock;
use std::collections::HashMap;
use std::sync::Arc;
use std::time::SystemTime;
use regex::Regex;

pub type BufferId = String;

#[derive(Clone)]
pub struct Buffer {
    pub id: BufferId,
    pub filepath: String,
    pub content: Vec<u8>,
    pub is_dirty: bool,
    pub is_read_only: bool,
    pub encoding: String,
    pub language_type: String,
    pub last_modified: SystemTime,

    undo_stack: Vec<Vec<u8>>,
    redo_stack: Vec<Vec<u8>>,
}

pub struct BufferManager {
    buffers: Arc<RwLock<HashMap<BufferId, Buffer>>>,
    active: Arc<RwLock<Option<BufferId>>>,
    event_bus: Option<EventBus>,
}

impl BufferManager {
    pub fn new(event_bus: Option<EventBus>) -> Self {
        Self {
            buffers: Arc::new(RwLock::new(HashMap::new())),
            active: Arc::new(RwLock::new(None)),
            event_bus,
        }
    }

    pub fn open_buffer(&self, filepath: &str, encoding: &str) -> Buffer {
        let mut buffers = self.buffers.write();
        let id = filepath.to_string();

        let buf = Buffer {
            id: id.clone(),
            filepath: filepath.to_string(),
            content: Vec::new(),
            is_dirty: false,
            is_read_only: false,
            encoding: encoding.to_string(),
            language_type: "TXT".to_string(),
            last_modified: SystemTime::now(),
            undo_stack: Vec::new(),
            redo_stack: Vec::new(),
        };

        buffers.insert(id.clone(), buf.clone());
        *self.active.write() = Some(id.clone());

        if let Some(eb) = &self.event_bus {
            eb.publish("BufferOpened", &buf);
        }

        buf
    }

    pub fn get_active_buffer(&self) -> Result<Buffer, &'static str> {
        let active_opt = self.active.read().clone();
        if let Some(active_id) = active_opt {
            let buffers = self.buffers.read();
            if let Some(buf) = buffers.get(&active_id) {
                return Ok(buf.clone());
            }
        }
        Err("no active buffer")
    }

    pub fn mark_dirty(&self, id: &BufferId) {
        let mut buffers = self.buffers.write();
        if let Some(buf) = buffers.get_mut(id) {
            buf.undo_stack.push(buf.content.clone());
            buf.redo_stack.clear();
            buf.is_dirty = true;
            if let Some(eb) = &self.event_bus {
                eb.publish("BufferChanged", buf);
            }
        }
    }

    pub fn update_content(&self, id: &BufferId, content: Vec<u8>) {
        self.mark_dirty(id);
        let mut buffers = self.buffers.write();
        if let Some(buf) = buffers.get_mut(id) {
            buf.content = content;
            if let Some(eb) = &self.event_bus {
                eb.publish("BufferChanged", buf);
            }
        }
    }

    pub fn undo(&self, id: &BufferId) -> Result<(), &'static str> {
        let mut buffers = self.buffers.write();
        let buf = buffers.get_mut(id).ok_or("buffer not found")?;

        if buf.undo_stack.is_empty() {
            return Err("nothing to undo");
        }

        buf.redo_stack.push(buf.content.clone());
        let last = buf.undo_stack.pop().unwrap();
        buf.content = last;

        if let Some(eb) = &self.event_bus {
            eb.publish("BufferChanged", buf);
        }

        Ok(())
    }

    pub fn redo(&self, id: &BufferId) -> Result<(), &'static str> {
        let mut buffers = self.buffers.write();
        let buf = buffers.get_mut(id).ok_or("buffer not found")?;

        if buf.redo_stack.is_empty() {
            return Err("nothing to redo");
        }

        buf.undo_stack.push(buf.content.clone());
        let last = buf.redo_stack.pop().unwrap();
        buf.content = last;

        if let Some(eb) = &self.event_bus {
            eb.publish("BufferChanged", buf);
        }

        Ok(())
    }

    pub fn close_buffer(&self, id: &BufferId) -> Result<(), &'static str> {
        let buf = {
            let mut buffers = self.buffers.write();
            let removed = buffers.remove(id).ok_or("buffer not found")?;

            let mut active = self.active.write();
            if active.as_deref() == Some(id.as_str()) {
                *active = buffers.keys().next().cloned();
            }
            removed
        };

        if let Some(eb) = &self.event_bus {
            eb.publish("BufferClosed", &buf);
        }

        Ok(())
    }

    pub fn convert_line_endings(&self, id: &BufferId, eol: &str) -> Result<(), &'static str> {
        let mut buffers = self.buffers.write();
        let buf = buffers.get_mut(id).ok_or("buffer not found")?;

        let content_str = String::from_utf8_lossy(&buf.content);
        let re = Regex::new(r"\r\n|\r|\n").unwrap();
        let new_content = re.replace_all(&content_str, eol).to_string();

        buf.content = new_content.into_bytes();
        buf.is_dirty = true;

        if let Some(eb) = &self.event_bus {
            eb.publish("BufferChanged", buf);
        }

        Ok(())
    }

    pub fn switch_to_buffer(&self, id: &BufferId) -> Result<(), &'static str> {
        let buffers = self.buffers.read();
        if !buffers.contains_key(id) {
            return Err("buffer not found");
        }
        *self.active.write() = Some(id.clone());
        if let Some(eb) = &self.event_bus {
            eb.publish("BufferActivated", id);
        }
        Ok(())
    }

    pub fn get_event_bus(&self) -> Option<EventBus> {
        self.event_bus.clone()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_buffer_manager_open() {
        let bm = BufferManager::new(None);
        let buf = bm.open_buffer("test.txt", "UTF-8");
        assert_eq!(buf.filepath, "test.txt");
        assert_eq!(buf.encoding, "UTF-8");
        assert_eq!(bm.get_active_buffer().unwrap().id, "test.txt");
    }
}
