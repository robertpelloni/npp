package plugins

import (
	"errors"
	"testing"
)

type MockPlugin struct {
	InitCalled     bool
	ShutdownCalled bool
}

func (m *MockPlugin) Name() string { return "MockTextFX" }
func (m *MockPlugin) Version() string { return "1.0.0" }
func (m *MockPlugin) Init() error {
	m.InitCalled = true
	return nil
}
func (m *MockPlugin) Shutdown() error {
	m.ShutdownCalled = true
	return nil
}

type FailPlugin struct{}

func (f *FailPlugin) Name() string { return "FailFX" }
func (f *FailPlugin) Version() string { return "0.1" }
func (f *FailPlugin) Init() error {
	return errors.New("initialization failed")
}
func (f *FailPlugin) Shutdown() error {
	return nil
}

func TestPluginManager(t *testing.T) {
	pm := NewManager()

	mockPlugin := &MockPlugin{}

	// Test successful load
	err := pm.LoadPlugin(mockPlugin)
	if err != nil {
		t.Fatalf("Failed to load plugin: %v", err)
	}
	if !mockPlugin.InitCalled {
		t.Error("Expected plugin Init() to be called")
	}

	// Test duplicate load
	err = pm.LoadPlugin(mockPlugin)
	if err == nil {
		t.Error("Expected error loading duplicate plugin")
	}

	// Test failed load
	failPlugin := &FailPlugin{}
	err = pm.LoadPlugin(failPlugin)
	if err == nil {
		t.Error("Expected error on failed initialization")
	}

	// Test successful unload
	err = pm.UnloadPlugin("MockTextFX")
	if err != nil {
		t.Fatalf("Failed to unload plugin: %v", err)
	}
	if !mockPlugin.ShutdownCalled {
		t.Error("Expected plugin Shutdown() to be called")
	}

	// Test unload non-existent
	err = pm.UnloadPlugin("GhostPlugin")
	if err == nil {
		t.Error("Expected error unloading non-existent plugin")
	}
}
