use parking_lot::RwLock;
use std::collections::HashMap;

pub trait Plugin: Send + Sync {
    fn name(&self) -> String;
    fn version(&self) -> String;
    fn init(&mut self) -> Result<(), String>;
    fn shutdown(&mut self) -> Result<(), String>;
}

pub struct Manager {
    plugins: RwLock<HashMap<String, Box<dyn Plugin>>>,
}

impl Manager {
    pub fn new() -> Self {
        Self {
            plugins: RwLock::new(HashMap::new()),
        }
    }

    pub fn load_plugin(&self, mut plugin: Box<dyn Plugin>) -> Result<(), String> {
        let name = plugin.name();

        let mut plugins = self.plugins.write();
        if plugins.contains_key(&name) {
            return Err(format!("plugin {} is already loaded", name));
        }

        println!("Initializing plugin: {} (v{})", name, plugin.version());
        plugin.init().map_err(|e| format!("failed to initialize plugin {}: {}", name, e))?;

        plugins.insert(name, plugin);
        Ok(())
    }

    pub fn unload_plugin(&self, name: &str) -> Result<(), String> {
        let mut plugins = self.plugins.write();

        let mut plugin = plugins.remove(name)
            .ok_or_else(|| format!("plugin {} is not loaded", name))?;

        println!("Shutting down plugin: {}", name);
        plugin.shutdown().map_err(|e| format!("failed to shutdown plugin {} cleanly: {}", name, e))?;

        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    struct MockPlugin {
        name: String,
        version: String,
        initialized: bool,
        shutdown: bool,
    }

    impl Plugin for MockPlugin {
        fn name(&self) -> String {
            self.name.clone()
        }

        fn version(&self) -> String {
            self.version.clone()
        }

        fn init(&mut self) -> Result<(), String> {
            self.initialized = true;
            Ok(())
        }

        fn shutdown(&mut self) -> Result<(), String> {
            self.shutdown = true;
            Ok(())
        }
    }

    #[test]
    fn test_load_plugin() {
        let manager = Manager::new();
        let plugin = MockPlugin {
            name: "TestPlugin".to_string(),
            version: "1.0.0".to_string(),
            initialized: false,
            shutdown: false,
        };

        assert!(manager.load_plugin(Box::new(plugin)).is_ok());

        // Cannot load twice
        let plugin2 = MockPlugin {
            name: "TestPlugin".to_string(),
            version: "1.0.0".to_string(),
            initialized: false,
            shutdown: false,
        };
        assert!(manager.load_plugin(Box::new(plugin2)).is_err());
    }

    #[test]
    fn test_unload_plugin() {
        let manager = Manager::new();
        let plugin = MockPlugin {
            name: "TestPlugin".to_string(),
            version: "1.0.0".to_string(),
            initialized: false,
            shutdown: false,
        };

        manager.load_plugin(Box::new(plugin)).unwrap();
        assert!(manager.unload_plugin("TestPlugin").is_ok());
        assert!(manager.unload_plugin("TestPlugin").is_err());
    }
}
