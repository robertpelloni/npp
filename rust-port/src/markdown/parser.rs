pub struct Parser;

impl Parser {
    pub fn new() -> Self {
        Self
    }

    pub fn render_to_html(&self, md: &[u8]) -> Result<Vec<u8>, String> {
        let mut html = Vec::new();
        html.extend_from_slice(b"<html><body><pre>\n");
        html.extend_from_slice(md);
        html.extend_from_slice(b"\n</pre></body></html>");
        Ok(html)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_markdown_to_html() {
        let parser = Parser::new();
        let markdown = b"# Hello\nThis is markdown.";
        let html = parser.render_to_html(markdown).unwrap();

        let expected = b"<html><body><pre>\n# Hello\nThis is markdown.\n</pre></body></html>";
        assert_eq!(html, expected);
    }
}
