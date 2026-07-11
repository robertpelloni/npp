package auth

import (
	"errors"

	"github.com/notepad-plus-plus/ultra-project/pkg/commands"
)

// TokenMiddleware validates the presence of a valid authentication token in the command arguments.
// Why: Before exposing command execution over WebSocket/HTTP or external RPC, we must ensure
//      the caller is authenticated. This middleware intercepts the command execution.
func TokenMiddleware(id string, next commands.CommandFunc) commands.CommandFunc {
	return func(args map[string]interface{}) error {
		// Bypass authentication for the login route
		if id == "Auth.Login" {
			return next(args)
		}
		if args == nil {
			return errors.New("unauthorized: missing token")
		}

		tokenRaw, ok := args["token"]
		if !ok {
			return errors.New("unauthorized: missing token")
		}

		token, ok := tokenRaw.(string)
		if !ok || !IsValidToken(token) {
			return errors.New("unauthorized: invalid token")
		}

		// Remove token from args before passing to the actual command to prevent pollution
		// (Optional, depending on if commands need to know about the token, but usually they don't)
		// delete(args, "token")

		return next(args)
	}
}
