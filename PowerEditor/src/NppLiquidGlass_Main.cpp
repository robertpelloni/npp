/**
 * NppLiquidGlass_Main.cpp — Liquid Glass Overhaul (v3.0)
 *
 * This version migrates from bobui (Qt6) to bobgui (GTK4/Libadwaita) 
 * to achieve 1:1 feature parity with Notepad++ and Geany with native
 * performance and a cohesive OS-level "Liquid Glass" appearance.
 */

#include <bobgui.h>
#include <windows.h>
#include "Scintilla.h"
#include "BobScintilla.h"
#include "GlassSettings.h"

// Note: This is the entry point for the GTK4-based "Liquid Glass" overhaul.
// We are mapping the existing NPP logic into the new GTK/Libadwaita widgets.

class BobNppGlassWindow {
public:
    BobNppGlassWindow() {
        m_app = adw_application_new("org.notepad-plus-plus.liquidglass", G_APPLICATION_DEFAULT_FLAGS);
        g_signal_connect(m_app, "activate", G_CALLBACK(on_activate), this);
    }

    int run(int argc, char** argv) {
        return g_application_run(G_APPLICATION(m_app), argc, argv);
    }

private:
    static void on_activate(AdwApplication* app, gpointer user_data) {
        auto* self = static_cast<BobNppGlassWindow*>(user_data);
        self->m_window = adw_application_window_new(app);
        gtk_window_set_title(GTK_WINDOW(self->m_window), "Notepad++ Liquid Glass v3");
        gtk_window_set_default_size(GTK_WINDOW(self->m_window), 1200, 800);

        // Header Bar
        auto* header = adw_header_bar_new();
        adw_application_window_set_content(ADW_APPLICATION_WINDOW(self->m_window), 
            GTK_WIDGET(adw_toolbar_view_new()));
        
        auto* view = ADW_TOOLBAR_VIEW(adw_application_window_get_content(ADW_APPLICATION_WINDOW(self->m_window)));
        adw_toolbar_view_add_top_bar(view, header);

        // Editor Area (Native Scintilla inside GTK Fixed/Viewport)
        // Note: GTK4 uses GSK for rendering. We wrap the HWND via GDK Win32.
        self->m_mainBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        adw_toolbar_view_set_content(view, self->m_mainBox);

        gtk_window_present(GTK_WINDOW(self->m_window));
    }

    AdwApplication* m_app;
    GtkWidget* m_window;
    GtkWidget* m_mainBox;
};

int main(int argc, char** argv) {
    BobNppGlassWindow app;
    return app.run(argc, argv);
}
