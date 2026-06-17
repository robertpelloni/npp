use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct AppConfig {
    pub skin_theme: String,
    pub enable_clear_type: bool,

    // Tab Management
    pub vertical_tabs_enabled: bool,
    pub vertical_tab_width: u32,

    // Typography
    pub mix_fonts: bool,

    // Editor Behavior
    pub disable_autocomplete_in_text: bool,
    pub autosave_enabled: bool,
}

impl Default for AppConfig {
    fn default() -> Self {
        Self {
            skin_theme: "ModernGlass".to_string(),
            enable_clear_type: true,
            vertical_tabs_enabled: true,
            vertical_tab_width: 200,
            mix_fonts: true,
            disable_autocomplete_in_text: true,
            autosave_enabled: true,
        }
    }
}
