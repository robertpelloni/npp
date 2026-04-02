# Notepad++ Ultimate Vision

## Core Philosophy
The ultimate goal of this project is to transform Notepad++ from a classic, highly functional text editor into an ultra-modern, incredibly robust, and feature-rich development environment, without sacrificing the speed and lightweight nature that made it famous.

## Key Pillars of the Vision

1. **Ultra-Modern User Interface**
   - Transition from the classic Win32 look to a "super modern glass" UI.
   - Implement comprehensive skinning capabilities allowing users to customize every visual aspect of the application.
   - Introduce fully functional vertical tabs to handle massive numbers of open documents efficiently.
   - Enable ClearType and advanced font rendering techniques globally.

2. **Absolute Data Security (Never Lose a File)**
   - Implement an "automatic versioning autosave" system.
   - The editor should silently keep all historical versions of all files edited.
   - Provide a seamless UI to scrub through a file's history and restore previous states.

3. **Core Feature Expansion (Geany Parity & Plugins)**
   - Achieve 1:1 feature parity with Geany (a lightweight IDE). This implies adding features like robust project management, built-in terminal emulators, advanced build system integration, and symbol navigation out-of-the-box.
   - Absorb popular, essential plugins directly into the core codebase to ensure stability and uniform UX. Specifically, integrate all functionalities of `textfx2`.

4. **Intelligent Context Awareness**
   - The editor must be smart enough to distinguish between code and prose seamlessly.
   - (Implemented) Intelligently disable code-centric features like aggressive autocomplete popups in standard non-code text files (Normal Text, Markdown, etc.).
   - Default styling should intuitively use proportional fonts for prose and monospaced fonts for code out-of-the-box (to be configured via the Style Configurator).
