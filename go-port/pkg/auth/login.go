package auth

import (
	"crypto/rand"
	"encoding/hex"
	"errors"
	"os"
	"sync"
)

var (
	validTokens = make(map[string]bool)
	tokenMu     sync.RWMutex
)

// GenerateToken creates a simple secure random string token.
// Why: In the absence of a full JWT standard library requirement, this provides
//      a basic secure token for session validation across WebSockets/RPC.
func GenerateToken() (string, error) {
	bytes := make([]byte, 16)
	if _, err := rand.Read(bytes); err != nil {
		return "", err
	}
	token := hex.EncodeToString(bytes)

	tokenMu.Lock()
	validTokens[token] = true
	tokenMu.Unlock()

	return token, nil
}

// IsValidToken checks if the provided token exists in our active sessions.
func IsValidToken(token string) bool {
	tokenMu.RLock()
	defer tokenMu.RUnlock()
	return validTokens[token]
}

// Authenticate is a command function that handles login requests.
// In a real scenario, this would check username/password and return the token via RPC response.
func Authenticate(args map[string]interface{}) error {
	if args == nil {
		return errors.New("missing credentials")
	}

	// Dummy credential check
	username, uOk := args["username"].(string)
	password, pOk := args["password"].(string)

	if !uOk || !pOk || username == "" || password == "" {
		return errors.New("invalid credentials format")
	}

	// TODO: Replace hardcoded dummy authentication with secure database lookup
	adminUser := os.Getenv("ADMIN_USERNAME")
	adminPass := os.Getenv("ADMIN_PASSWORD")

	if adminUser == "" || adminPass == "" {
		return errors.New("authentication not configured on server")
	}

	if username == adminUser && password == adminPass {
				token, err := GenerateToken()
		if err != nil {
			return err
		}
		args["response_token"] = token
		return nil
	}

	return errors.New("authentication failed")
}
