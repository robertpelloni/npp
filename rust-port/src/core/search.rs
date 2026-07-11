use regex::Regex;

pub struct SearchResult {
    pub start: usize,
    pub end: usize,
}

pub struct SearchService;

impl SearchService {
    pub fn new() -> Self {
        Self
    }

    pub fn find_all(&self, content: &[u8], query: &str) -> Vec<SearchResult> {
        let mut results = Vec::new();
        let query_bytes = query.as_bytes();
        let mut start = 0;

        while let Some(idx) = content[start..]
            .windows(query_bytes.len())
            .position(|window| window == query_bytes)
        {
            let match_start = start + idx;
            results.push(SearchResult {
                start: match_start,
                end: match_start + query_bytes.len(),
            });
            start = match_start + query_bytes.len();
        }

        results
    }

    pub fn find_all_regex(&self, content: &[u8], pattern: &str) -> Result<Vec<SearchResult>, regex::Error> {
        let content_str = String::from_utf8_lossy(content);
        let re = Regex::new(pattern)?;
        let mut results = Vec::new();

        for mat in re.find_iter(&content_str) {
            results.push(SearchResult {
                start: mat.start(),
                end: mat.end(),
            });
        }

        Ok(results)
    }

    pub fn replace_all(&self, content: &[u8], query: &str, replacement: &str) -> Vec<u8> {
        let content_str = String::from_utf8_lossy(content);
        content_str.replace(query, replacement).into_bytes()
    }

    pub fn replace_all_regex(&self, content: &[u8], pattern: &str, replacement: &str) -> Result<Vec<u8>, regex::Error> {
        let content_str = String::from_utf8_lossy(content);
        let re = Regex::new(pattern)?;
        Ok(re.replace_all(&content_str, replacement).into_owned().into_bytes())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_find_all() {
        let ss = SearchService::new();
        let results = ss.find_all(b"hello world hello", "hello");
        assert_eq!(results.len(), 2);
        assert_eq!(results[0].start, 0);
        assert_eq!(results[1].start, 12);
    }
}
