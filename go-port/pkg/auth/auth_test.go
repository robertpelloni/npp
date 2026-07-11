package auth

import (
	"os"
	"testing"

)

func TestLoginGeneration(t *testing.T) {
		os.Setenv("ADMIN_USERNAME", "admin")
	os.Setenv("ADMIN_PASSWORD", "password123")
	defer os.Unsetenv("ADMIN_USERNAME")
	defer os.Unsetenv("ADMIN_PASSWORD")

	// Test valid login
	args := map[string]interface{}{
		"username": "admin",
		"password": "password123",
	}
	err := Authenticate(args)
	if err != nil {
		t.Fatalf("expected successful auth, got error: %v", err)
	}
	token, ok := args["response_token"].(string)
	if !ok || token == "" {
		t.Fatal("expected token in args, got missing or empty")
	}
	if !IsValidToken(token) {
		t.Fatal("generated token should be valid")
	}

	// Test invalid login
	err = Authenticate(map[string]interface{}{
		"username": "admin",
		"password": "wrongpassword",
	})
	if err == nil {
		t.Fatal("expected authentication failure")
	}
}

func TestTokenValidation(t *testing.T) {
	// Dummy command to wrap
	executed := false
	dummyCmd := func(args map[string]interface{}) error {
		executed = true
		return nil
	}

	wrappedCmd := TokenMiddleware("Some.Command", dummyCmd)

	// Test missing token
	err := wrappedCmd(map[string]interface{}{})
	if err == nil || err.Error() != "unauthorized: missing token" {
		t.Fatalf("expected missing token error, got: %v", err)
	}
	if executed {
		t.Fatal("command should not execute without token")
	}


	// Test Auth.Login bypass
	bypassCmd := TokenMiddleware("Auth.Login", dummyCmd)
	err = bypassCmd(map[string]interface{}{})
	if err != nil {
		t.Fatalf("expected Auth.Login to bypass missing token error, got: %v", err)
	}
	executed = false
	// Test invalid token
	err = wrappedCmd(map[string]interface{}{
		"token": "invalid_token_123",
	})
	if err == nil || err.Error() != "unauthorized: invalid token" {
		t.Fatalf("expected invalid token error, got: %v", err)
	}
	if executed {
		t.Fatal("command should not execute with invalid token")
	}

	// Test valid token
	token, _ := GenerateToken()
	err = wrappedCmd(map[string]interface{}{
		"token": token,
	})
	if err != nil {
		t.Fatalf("expected successful execution, got error: %v", err)
	}
	if !executed {
		t.Fatal("command should execute with valid token")
	}
}
