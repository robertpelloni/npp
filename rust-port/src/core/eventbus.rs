use parking_lot::RwLock;
use std::collections::HashMap;
use std::sync::Arc;

pub type EventCallback = Box<dyn Fn(&dyn std::any::Any) + Send + Sync>;

#[derive(Clone)]
pub struct EventBus {
    listeners: Arc<RwLock<HashMap<String, Vec<Arc<EventCallback>>>>>,
}

impl EventBus {
    pub fn new() -> Self {
        Self {
            listeners: Arc::new(RwLock::new(HashMap::new())),
        }
    }

    pub fn subscribe<F>(&self, event_id: &str, callback: F)
    where
        F: Fn(&dyn std::any::Any) + Send + Sync + 'static,
    {
        let mut listeners = self.listeners.write();
        listeners
            .entry(event_id.to_string())
            .or_default()
            .push(Arc::new(Box::new(callback)));
    }

    pub fn publish(&self, event_id: &str, payload: &dyn std::any::Any) {
        let listeners = self.listeners.read();
        if let Some(callbacks) = listeners.get(event_id) {
            for cb in callbacks {
                cb(payload);
            }
        }
    }
}
