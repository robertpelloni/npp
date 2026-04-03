# Immediate Tasks
- [ ] Build a `BobScintilla` widget class that subclasses `QWidget` and hosts the native Scintilla HWND.
- [ ] Wire the `Scintilla_RegisterClasses` call in `main()`.
- [ ] Replace `QPlainTextEdit` with `BobScintilla` inside `GlassEditorPanel`.
- [ ] Test alpha-blending `SCI_SETCARETLINEBACKALPHA` with Liquid Glass DWM backdrop.
- [ ] Wire `GlassEditorPanel`'s text operations (Undo, Redo, Cut, Copy, Paste) to `BobScintilla` SCI messages instead of `QPlainTextEdit` slots.
