use parking_lot::RwLock;
use crate::core::buffer::BufferId;
use std::sync::Arc;

#[derive(Clone, PartialEq, Debug)]
pub enum TabPlacement {
    Top,
    Vertical,
}

#[derive(Clone, Debug)]
pub struct Tab {
    pub buffer_id: BufferId,
    pub title: String,
    pub is_active: bool,
}

pub struct Layout {
    pub placement: Arc<RwLock<TabPlacement>>,
    pub vertical_width_px: Arc<RwLock<u32>>,
    pub tabs: Arc<RwLock<Vec<Tab>>>,
}

impl Layout {
    pub fn new() -> Self {
        Self {
            placement: Arc::new(RwLock::new(TabPlacement::Vertical)), // Fulfilling user requirement as default
            vertical_width_px: Arc::new(RwLock::new(300)),
            tabs: Arc::new(RwLock::new(Vec::new())),
        }
    }

    pub fn add_tab(&self, buf_id: BufferId, title: String) {
        let mut tabs = self.tabs.write();

        for tab in tabs.iter_mut() {
            tab.is_active = false;
        }

        tabs.push(Tab {
            buffer_id: buf_id,
            title,
            is_active: true,
        });
    }

    pub fn set_vertical_width(&self, width: u32) {
        if width > 50 && width < 1000 {
            *self.vertical_width_px.write() = width;
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_layout_default_vertical() {
        let layout = Layout::new();
        assert_eq!(*layout.placement.read(), TabPlacement::Vertical);
    }

    #[test]
    fn test_add_tab() {
        let layout = Layout::new();
        layout.add_tab("1".to_string(), "test.txt".to_string());

        let tabs = layout.tabs.read();
        assert_eq!(tabs.len(), 1);
        assert!(tabs[0].is_active);
    }
}
