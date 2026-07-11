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

impl AppConfig {
    pub fn save_to_file(&self, filepath: &str) -> Result<(), Box<dyn std::error::Error>> {
        let json = serde_json::to_string_pretty(self)?;
        std::fs::write(filepath, json)?;
        Ok(())
    }

    pub fn load_from_file(filepath: &str) -> Result<Self, Box<dyn std::error::Error>> {
        let json = std::fs::read_to_string(filepath)?;
        let config: AppConfig = serde_json::from_str(&json)?;
        Ok(config)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::fs;

    #[test]
    fn test_save_and_load_config() {
        let config = AppConfig::default();
        let filepath = "test_config.json";

        config.save_to_file(filepath).unwrap();

        let loaded_config = AppConfig::load_from_file(filepath).unwrap();
        assert_eq!(config.skin_theme, loaded_config.skin_theme);
        assert_eq!(config.vertical_tabs_enabled, loaded_config.vertical_tabs_enabled);

        fs::remove_file(filepath).unwrap();
    }
}
