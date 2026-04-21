# Global Agent Instructions

These instructions govern all AI models (Claude, GPT, Gemini, Jules) operating within this repository.

## General Directives
*   **Documentation is King:** You must update `ROADMAP.md`, `TODO.md`, `CHANGELOG.md`, `MEMORY.md`, and `HANDOFF.md` at the end of every task or session. Input information must be documented in comprehensive, thorough, FULL extreme detail.
*   **Version Control:** Every build/commit must have a new version number updated across the project, centrally managed via `VERSION` and `CHANGELOG.md`. Version bumps *must* be referenced in commit messages.
*   **Deep Commenting:** Any code you write or touch requires deep, in-depth comments explaining the 'what', 'why', side effects, bugs, optimizations, and alternatives. Do not comment self-explanatory code just for the sake of it, but err on the side of extreme detail for business logic.
*   **Architecture Goals:** Keep the ultimate vision in mind: Decouple Win32, support Qt/GTK submodules, port to Go, and achieve 100% 1:1 Geany parity with superior implementations.
*   **Proactivity:** Correct errors you find along the way. Continue researching and documenting findings. If you can complete a feature, commit and push, and continue development without stopping, do so.

## Code Standards
*   **UI:** Do not hardcode font styles in `stylers.model.xml`.
*   **Submodules:** Meticulously manage submodules. Maintain documentation of all submodules with versions and locations.

## Model-Specific Instructions
*(To be detailed in specific files if needed, but refer to this file as the master source).*
