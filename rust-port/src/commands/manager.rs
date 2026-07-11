use parking_lot::RwLock;
use std::collections::HashMap;
use std::sync::Arc;

pub type CommandArgs = HashMap<String, String>;
pub type CommandFunc = Box<dyn Fn(CommandArgs) -> Result<(), String> + Send + Sync>;

pub struct Command {
    pub id: String,
    pub description: String,
    pub execute: CommandFunc,
}

#[derive(Clone)]
pub struct Manager {
    commands: Arc<RwLock<HashMap<String, Arc<Command>>>>,
}

impl Manager {
    pub fn new() -> Self {
        Self {
            commands: Arc::new(RwLock::new(HashMap::new())),
        }
    }

    pub fn register(&self, cmd: Command) {
        self.commands.write().insert(cmd.id.clone(), Arc::new(cmd));
    }

    pub fn execute(&self, id: &str, args: CommandArgs) -> Result<(), String> {
        let cmd = {
            let commands = self.commands.read();
            commands.get(id).cloned()
        };

        match cmd {
            Some(c) => (c.execute)(args),
            None => Err(format!("command not found: {}", id)),
        }
    }
}
