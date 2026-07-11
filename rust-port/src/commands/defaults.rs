use crate::commands::manager::{Command, Manager};
use crate::core::buffer::BufferManager;
use crate::core::search::SearchService;
use crate::config::parameters::AppConfig;
use crate::workspace::layout::{Layout, TabPlacement};
use crate::io::fileio::FileManager;
use std::sync::Arc;
use parking_lot::Mutex;

pub fn register_default_commands(
    manager: &Manager,
    buf_manager: Arc<BufferManager>,
    app_config: Arc<Mutex<AppConfig>>,
    layout: Arc<Layout>,
) {
    let new_file_count = Arc::new(Mutex::new(0));

    let bm_clone = Arc::clone(&buf_manager);
    let count_clone = Arc::clone(&new_file_count);
    let layout_clone_new = Arc::clone(&layout);

    manager.register(Command {
        id: "File.New".to_string(),
        description: "Create a new empty buffer".to_string(),
        execute: Box::new(move |_args| {
            let mut count = count_clone.lock();
            *count += 1;
            let title = format!("New {}", count);
            let buf = bm_clone.open_buffer(&title, "UTF-8");
            layout_clone_new.add_tab(buf.id, title);
            Ok(())
        }),
    });

    let bm_clone_open = Arc::clone(&buf_manager);
    let layout_clone_open = Arc::clone(&layout);
    manager.register(Command {
        id: "File.Open".to_string(),
        description: "Open an existing file".to_string(),
        execute: Box::new(move |args| {
            if let Some(filepath) = args.get("filepath") {
                let file_manager = FileManager::new();
                let content = file_manager.read_file(filepath).map_err(|e| e.to_string())?;
                let buf = bm_clone_open.open_buffer(filepath, "UTF-8");
                bm_clone_open.update_content(&buf.id, content);
                layout_clone_open.add_tab(buf.id, filepath.clone());
            }
            Ok(())
        }),
    });

    let bm_clone_save = Arc::clone(&buf_manager);
    manager.register(Command {
        id: "File.Save".to_string(),
        description: "Save the current buffer".to_string(),
        execute: Box::new(move |_args| {
            let buf = bm_clone_save.get_active_buffer()?;
            let file_manager = FileManager::new();
            file_manager.write_file(&buf.filepath, &buf.content).map_err(|e| e.to_string())?;
            // A real implementation would also mark is_dirty false here via a new method on BufferManager
            Ok(())
        }),
    });

    let bm_clone_close_all = Arc::clone(&buf_manager);
    let layout_clone_close_all = Arc::clone(&layout);
    manager.register(Command {
        id: "File.CloseAll".to_string(),
        description: "Close all open buffers".to_string(),
        execute: Box::new(move |_args| {
            while let Ok(buf) = bm_clone_close_all.get_active_buffer() {
                let _ = bm_clone_close_all.close_buffer(&buf.id);
            }
            layout_clone_close_all.tabs.write().clear();
            Ok(())
        }),
    });

    let bm_clone_undo = Arc::clone(&buf_manager);
    manager.register(Command {
        id: "Edit.Undo".to_string(),
        description: "Undo the last action".to_string(),
        execute: Box::new(move |_args| {
            let buf = bm_clone_undo.get_active_buffer()?;
            bm_clone_undo.undo(&buf.id).map_err(|e| e.to_string())
        }),
    });

    let bm_clone_redo = Arc::clone(&buf_manager);
    manager.register(Command {
        id: "Edit.Redo".to_string(),
        description: "Redo the last undone action".to_string(),
        execute: Box::new(move |_args| {
            let buf = bm_clone_redo.get_active_buffer()?;
            bm_clone_redo.redo(&buf.id).map_err(|e| e.to_string())
        }),
    });

    let app_config_clone = Arc::clone(&app_config);
    let layout_clone_toggle = Arc::clone(&layout);
    manager.register(Command {
        id: "View.ToggleVerticalTabs".to_string(),
        description: "Toggle vertical tab layout".to_string(),
        execute: Box::new(move |_args| {
            let mut config = app_config_clone.lock();
            config.vertical_tabs_enabled = !config.vertical_tabs_enabled;
            let mut placement = layout_clone_toggle.placement.write();
            *placement = if config.vertical_tabs_enabled {
                TabPlacement::Vertical
            } else {
                TabPlacement::Top
            };
            Ok(())
        }),
    });

    let bm_clone_unix = Arc::clone(&buf_manager);
    manager.register(Command {
        id: "Format.ToUnix".to_string(),
        description: "Convert line endings to Unix (LF)".to_string(),
        execute: Box::new(move |_args| {
            let buf = bm_clone_unix.get_active_buffer()?;
            bm_clone_unix.mark_dirty(&buf.id);
            bm_clone_unix.convert_line_endings(&buf.id, "\n").map_err(|e| e.to_string())
        }),
    });

    let bm_clone_dos = Arc::clone(&buf_manager);
    manager.register(Command {
        id: "Format.ToDOS".to_string(),
        description: "Convert line endings to DOS (CRLF)".to_string(),
        execute: Box::new(move |_args| {
            let buf = bm_clone_dos.get_active_buffer()?;
            bm_clone_dos.mark_dirty(&buf.id);
            bm_clone_dos.convert_line_endings(&buf.id, "\r\n").map_err(|e| e.to_string())
        }),
    });

    let bm_clone_mac = Arc::clone(&buf_manager);
    manager.register(Command {
        id: "Format.ToMac".to_string(),
        description: "Convert line endings to Mac (CR)".to_string(),
        execute: Box::new(move |_args| {
            let buf = bm_clone_mac.get_active_buffer()?;
            bm_clone_mac.mark_dirty(&buf.id);
            bm_clone_mac.convert_line_endings(&buf.id, "\r").map_err(|e| e.to_string())
        }),
    });

    manager.register(Command {
        id: "Settings.Preferences".to_string(),
        description: "Show preference dialog".to_string(),
        execute: Box::new(move |_args| Ok(())),
    });

    let app_config_skin = Arc::clone(&app_config);
    manager.register(Command {
        id: "Settings.SetSkin".to_string(),
        description: "Change the UI skin theme".to_string(),
        execute: Box::new(move |args| {
            if let Some(skin) = args.get("skin") {
                app_config_skin.lock().skin_theme = skin.clone();
            }
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
        let app_config = Arc::new(Mutex::new(AppConfig::default()));
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

    #[test]
    fn test_edit_commands() {
        let event_bus = EventBus::new();
        let buf_manager = Arc::new(BufferManager::new(Some(event_bus)));
        let app_config = Arc::new(Mutex::new(AppConfig::default()));
        let layout = Arc::new(Layout::new());
        let cmd_manager = Manager::new();

        register_default_commands(&cmd_manager, buf_manager.clone(), app_config, layout);

        let buf = buf_manager.open_buffer("test.txt", "UTF-8");
        buf_manager.update_content(&buf.id, b"state 1".to_vec());
        buf_manager.update_content(&buf.id, b"state 2".to_vec());

        assert!(cmd_manager.execute("Edit.Undo", HashMap::new()).is_ok());
        let active_buf = buf_manager.get_active_buffer().unwrap();
        assert_eq!(String::from_utf8_lossy(&active_buf.content), "state 1");

        assert!(cmd_manager.execute("Edit.Redo", HashMap::new()).is_ok());
        let active_buf2 = buf_manager.get_active_buffer().unwrap();
        assert_eq!(String::from_utf8_lossy(&active_buf2.content), "state 2");
    }

    #[test]
    fn test_view_commands() {
        let event_bus = EventBus::new();
        let buf_manager = Arc::new(BufferManager::new(Some(event_bus)));
        let app_config = Arc::new(Mutex::new(AppConfig::default()));
        let layout = Arc::new(Layout::new());
        let cmd_manager = Manager::new();

        register_default_commands(&cmd_manager, buf_manager.clone(), app_config.clone(), layout.clone());

        assert_eq!(*layout.placement.read(), TabPlacement::Vertical);

        assert!(cmd_manager.execute("View.ToggleVerticalTabs", HashMap::new()).is_ok());
        assert_eq!(*layout.placement.read(), TabPlacement::Top);
        assert_eq!(app_config.lock().vertical_tabs_enabled, false);
    }
}
