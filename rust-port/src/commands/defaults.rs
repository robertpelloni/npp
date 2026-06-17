use crate::commands::manager::{Command, Manager};
use crate::core::buffer::BufferManager;
use crate::config::parameters::AppConfig;
use crate::workspace::layout::Layout;
use std::sync::Arc;
use parking_lot::Mutex;

pub fn register_default_commands(
    manager: &Manager,
    buf_manager: Arc<BufferManager>,
    _app_config: Arc<AppConfig>,
    _layout: Arc<Layout>,
) {
    let new_file_count = Arc::new(Mutex::new(0));

    let bm_clone = Arc::clone(&buf_manager);
    let count_clone = Arc::clone(&new_file_count);

    manager.register(Command {
        id: "File.New".to_string(),
        description: "Create a new empty buffer".to_string(),
        execute: Box::new(move |_args| {
            let mut count = count_clone.lock();
            *count += 1;
            let title = format!("New {}", count);
            bm_clone.open_buffer(&title, "UTF-8");
            Ok(())
        }),
    });
}
