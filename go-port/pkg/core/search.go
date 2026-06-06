package core

import (
	"bytes"
	"regexp"
)

// SearchResult represents a single match in the buffer
type SearchResult struct {
	Start int
	End   int
}

// SearchService provides string and regex search capabilities
type SearchService struct{}

func NewSearchService() *SearchService {
	return &SearchService{}
}

// FindAll occurrences of a literal string in the given content
func (s *SearchService) FindAll(content []byte, query string) []SearchResult {
	results := make([]SearchResult, 0)
	queryBytes := []byte(query)
	start := 0

	for {
		idx := bytes.Index(content[start:], queryBytes)
		if idx == -1 {
			break
		}

		matchStart := start + idx
		results = append(results, SearchResult{
			Start: matchStart,
			End:   matchStart + len(queryBytes),
		})
		start = matchStart + len(queryBytes)
	}

	return results
}

// FindAllRegex finds all matches of a regex pattern in the content
func (s *SearchService) FindAllRegex(content []byte, pattern string) ([]SearchResult, error) {
	re, err := regexp.Compile(pattern)
	if err != nil {
		return nil, err
	}

	matches := re.FindAllIndex(content, -1)
	results := make([]SearchResult, len(matches))
	for i, m := range matches {
		results[i] = SearchResult{
			Start: m[0],
			End:   m[1],
		}
	}

	return results, nil
}
