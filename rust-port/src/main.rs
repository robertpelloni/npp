pub mod core;
pub mod workspace;
pub mod textfx;
pub mod commands;
pub mod markdown;
pub mod config;
pub mod io;
pub mod autosave;
pub mod lsp;

use std::sync::Arc;
use std::collections::HashMap;

use crate::core::eventbus::EventBus;
use crate::core::buffer::BufferManager;
use crate::config::parameters::AppConfig;
use crate::workspace::layout::Layout;
use crate::commands::manager::Manager as CommandManager;
use crate::commands::defaults::register_default_commands;
use crate::lsp::manager::Manager as LspManager;
use crate::textfx::core::TextFx;

fn main() {
    println!("Initializing Notepad++ Ultra-Project (Rust Backend)...");

    // Initialize Core Systems
    let event_bus = EventBus::new();
    let buf_manager = Arc::new(BufferManager::new(Some(event_bus.clone())));
    let app_config = Arc::new(parking_lot::Mutex::new(AppConfig::default()));
    let layout = Arc::new(Layout::new());

    // Initialize LSP Manager
    let _lsp_manager = Arc::new(LspManager::new());

    // Initialize Command Router
    let cmd_manager = CommandManager::new();
    register_default_commands(&cmd_manager, buf_manager.clone(), app_config.clone(), layout.clone());

    let text_fx = TextFx::new();
    text_fx.register_commands(&cmd_manager);

    println!("Core data models and command router initialized.");

    println!("Simulating UI Action: File.New");
    let args = HashMap::new();
    if let Err(e) = cmd_manager.execute("File.New", args) {
        eprintln!("Command failed: {}", e);
        std::process::exit(1);
    }

    if let Ok(active_buf) = buf_manager.get_active_buffer() {
        println!("Active buffer is now: {}", active_buf.id);
    }

    println!("Backend initialized successfully. Awaiting Native UI hooks.");
}
