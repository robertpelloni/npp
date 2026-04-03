---
active: true
session_id: "npp-bobui-overhaul"
workflow_mode: "standard"
task_complexity: "complex"
current_phase: 1
phases:
  - id: 1
    name: "Design"
    agent: "architect"
    status: "completed"
  - id: 2
    name: "Infrastructure (Phase 2.1)"
    agent: "architect"
    status: "completed"
    tasks:
      - id: 2.1.1
        name: "Root CMake Configuration"
        status: "completed"
      - id: 2.1.2
        name: "App Entry Point Scaffolding"
        status: "completed"
      - id: 2.1.3
        name: "Build Verification"
        status: "completed"
  - id: 3
    name: "Core UI Scaffolding (Phase 2.2)"
    agent: "coder"
    status: "in_progress"
  - id: 3
    name: "Implementation (Core)"
    agent: "coder"
    status: "pending"
  - id: 4
    name: "TextFX Integration"
    agent: "coder"
    status: "pending"
  - id: 5
    name: "Validation & QA"
    agent: "tester"
    status: "pending"
---

# Maestro Session: NPP Overhaul to BobUI

## Objective
Overhaul the Notepad++ UI layer to use the `bobui` framework (Qt6 equivalent) and natively integrate all TextFX plugin features.

## Phase 1: Design
- Map Win32 NPP components to BobUI (Qt) equivalents.
- Design the plugin bridge for existing NPP plugins.
- Architect the native TextFX feature set.
