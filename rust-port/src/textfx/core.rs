use crate::commands::manager::{Command, Manager};

pub struct TextFx;

impl TextFx {
    pub fn new() -> Self {
        Self
    }

    pub fn sort_lines(&self, _case_sensitive: bool) -> Result<(), String> {
        // Stub logic for modifying the active buffer
        Ok(())
    }

    pub fn register_commands(&self, manager: &Manager) {
        manager.register(Command {
            id: "TextFX.SortLinesCaseInsensitive".to_string(),
            description: "Sort selected lines ignoring case".to_string(),
            execute: Box::new(|_args| {
                Ok(())
            }),
        });

        manager.register(Command {
            id: "TextFX.SortLinesCaseSensitive".to_string(),
            description: "Sort selected lines strictly by case".to_string(),
            execute: Box::new(|_args| {
                Ok(())
            }),
        });
    }
}

pub fn sort_string_lines(input: &str, case_sensitive: bool) -> String {
    let mut lines: Vec<&str> = input.lines().collect();

    if case_sensitive {
        lines.sort();
    } else {
        lines.sort_by(|a, b| a.to_lowercase().cmp(&b.to_lowercase()));
    }

    lines.join("\n")
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_sort_string_lines_sensitive() {
        let input = "b\nA\na\nB";
        let expected = "A\nB\na\nb";
        assert_eq!(sort_string_lines(input, true), expected);
    }

    #[test]
    fn test_sort_string_lines_insensitive() {
        let input = "b\nA\na\nB";
        let expected = "A\na\nb\nB";
        assert_eq!(sort_string_lines(input, false), expected);
    }
}
