use crate::commands::manager::{Command, Manager};
use crate::core::buffer::BufferManager;
use crate::core::search::SearchService;
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

    let bm_clone2 = Arc::clone(&buf_manager);
    manager.register(Command {
        id: "Search.Find".to_string(),
        description: "Find literal string in current buffer".to_string(),
        execute: Box::new(move |args| {
            let query = args.get("query").ok_or("missing query")?;
            let active_buf = bm_clone2.get_active_buffer()?;
            let search_service = SearchService::new();
            let results = search_service.find_all(&active_buf.content, query);

            // In a real implementation, results would be published to the EventBus
            // e.g. bm_clone2.get_event_bus().unwrap().publish("Search.Results", &results);
            let _ = results;
            Ok(())
        }),
    });

    let bm_clone3 = Arc::clone(&buf_manager);
    manager.register(Command {
        id: "Search.Replace".to_string(),
        description: "Replace literal string in current buffer".to_string(),
        execute: Box::new(move |args| {
            let query = args.get("query").ok_or("missing query")?;
            let replacement = args.get("replacement").ok_or("missing replacement")?;

            let active_buf = bm_clone3.get_active_buffer()?;
            let search_service = SearchService::new();
            let new_content = search_service.replace_all(&active_buf.content, query, replacement);

            bm_clone3.update_content(&active_buf.id, new_content);
            Ok(())
        }),
    });
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::collections::HashMap;
    use crate::core::eventbus::EventBus;

    #[test]
    fn test_search_commands() {
        let event_bus = EventBus::new();
        let buf_manager = Arc::new(BufferManager::new(Some(event_bus)));
        let app_config = Arc::new(AppConfig::default());
        let layout = Arc::new(Layout::new());
        let cmd_manager = Manager::new();

        register_default_commands(&cmd_manager, buf_manager.clone(), app_config, layout);

        // Setup buffer
        let buf = buf_manager.open_buffer("test.txt", "UTF-8");
        buf_manager.update_content(&buf.id, b"hello world hello".to_vec());

        // Test Find
        let mut find_args = HashMap::new();
        find_args.insert("query".to_string(), "hello".to_string());
        assert!(cmd_manager.execute("Search.Find", find_args).is_ok());

        // Test Replace
        let mut replace_args = HashMap::new();
        replace_args.insert("query".to_string(), "hello".to_string());
        replace_args.insert("replacement".to_string(), "hi".to_string());
        assert!(cmd_manager.execute("Search.Replace", replace_args).is_ok());

        let active_buf = buf_manager.get_active_buffer().unwrap();
        assert_eq!(String::from_utf8_lossy(&active_buf.content), "hi world hi");
    }
}
