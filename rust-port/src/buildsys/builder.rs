use std::io::{BufRead, BufReader};
use std::process::{Command, Stdio};
use std::thread;

use std::sync::Arc;

pub type BuildOutputCallback = Arc<dyn Fn(String) + Send + Sync>;

pub struct BuildManager {}

impl BuildManager {
    pub fn new() -> Self {
        Self {}
    }

    pub fn execute_build(
        &self,
        command: &str,
        args: &[&str],
        working_dir: &str,
        on_output: BuildOutputCallback,
    ) -> Result<(), String> {
        let mut cmd = Command::new(command);
        cmd.args(args)
            .current_dir(working_dir)
            .stdout(Stdio::piped())
            .stderr(Stdio::piped());

        let mut child = cmd.spawn().map_err(|e| format!("failed to start build command: {}", e))?;

        let stdout = child.stdout.take().ok_or("failed to get stdout pipe")?;
        let stderr = child.stderr.take().ok_or("failed to get stderr pipe")?;

        let on_output_out = Arc::clone(&on_output);
        let stdout_handle = thread::spawn(move || {
            let reader = BufReader::new(stdout);
            for line in reader.lines() {
                if let Ok(line) = line {
                    on_output_out(line);
                }
            }
        });

        let on_output_err = Arc::clone(&on_output);
        let stderr_handle = thread::spawn(move || {
            let reader = BufReader::new(stderr);
            for line in reader.lines() {
                if let Ok(line) = line {
                    on_output_err(format!("ERROR: {}", line));
                }
            }
        });

        stdout_handle.join().map_err(|_| "stdout thread panicked")?;
        stderr_handle.join().map_err(|_| "stderr thread panicked")?;

        let status = child.wait().map_err(|e| format!("failed to wait on child: {}", e))?;

        if !status.success() {
            return Err(format!("build command exited with error status: {}", status));
        }

        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::sync::{Arc, Mutex};

    #[test]
    fn test_execute_build() {
        let bm = BuildManager::new();
        let output = Arc::new(Mutex::new(Vec::new()));

        let output_clone = Arc::clone(&output);
        let callback = Arc::new(move |line: String| {
            output_clone.lock().unwrap().push(line);
        });

        let result = bm.execute_build("echo", &["hello build"], ".", callback);
        assert!(result.is_ok());

        let out = output.lock().unwrap();
        assert_eq!(out.len(), 1);
        assert_eq!(out[0], "hello build");
    }

    #[test]
    fn test_execute_build_error() {
        let bm = BuildManager::new();
        let callback = Arc::new(move |_line: String| {});

        let result = bm.execute_build("false", &[], ".", callback);
        assert!(result.is_err());
    }
}
