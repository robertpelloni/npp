/**
 * GlassCompareManager.h — Side-by-side Document Comparison for BobUI
 */

#ifndef GLASS_COMPARE_MANAGER_H
#define GLASS_COMPARE_MANAGER_H

#include "BobScintilla.h"
#include <QStringList>
#include <QVector>

class GlassCompareManager {
public:
    static void compare(BobScintilla* ed1, BobScintilla* ed2) {
        if (!ed1 || !ed2) return;

        QStringList lines1 = ed1->text().split('\n');
        QStringList lines2 = ed2->text().split('\n');

        // Clear previous markers (Marker ID 10 for addition, 11 for deletion, 12 for change)
        ed1->send(SCI_MARKERDELETEALL, 10);
        ed1->send(SCI_MARKERDELETEALL, 11);
        ed1->send(SCI_MARKERDELETEALL, 12);
        ed2->send(SCI_MARKERDELETEALL, 10);
        ed2->send(SCI_MARKERDELETEALL, 11);
        ed2->send(SCI_MARKERDELETEALL, 12);

        // Define markers
        setupMarkers(ed1);
        setupMarkers(ed2);

        // Simple line-by-line comparison
        int maxLines = std::max(lines1.size(), lines2.size());
        for (int i = 0; i < maxLines; ++i) {
            QString l1 = (i < lines1.size()) ? lines1[i] : "";
            QString l2 = (i < lines2.size()) ? lines2[i] : "";

            if (l1 != l2) {
                if (i >= lines1.size()) {
                    ed2->send(SCI_MARKERADD, i, 10); // Added in 2
                } else if (i >= lines2.size()) {
                    ed1->send(SCI_MARKERADD, i, 11); // Deleted from 2
                } else {
                    ed1->send(SCI_MARKERADD, i, 12); // Changed
                    ed2->send(SCI_MARKERADD, i, 12);
                }
            }
        }
    }

    static void clear(BobScintilla* ed) {
        if (!ed) return;
        ed->send(SCI_MARKERDELETEALL, 10);
        ed->send(SCI_MARKERDELETEALL, 11);
        ed->send(SCI_MARKERDELETEALL, 12);
    }

private:
    static void setupMarkers(BobScintilla* ed) {
        // Marker 10: Addition (Green)
        ed->send(SCI_MARKERDEFINE, 10, SC_MARK_FULLRECT);
        ed->send(SCI_MARKERSETBACK, 10, 0x004400); // Dark Green BGR

        // Marker 11: Deletion (Red)
        ed->send(SCI_MARKERDEFINE, 11, SC_MARK_FULLRECT);
        ed->send(SCI_MARKERSETBACK, 11, 0x000044); // Dark Red BGR

        // Marker 12: Change (Yellow)
        ed->send(SCI_MARKERDEFINE, 12, SC_MARK_FULLRECT);
        ed->send(SCI_MARKERSETBACK, 12, 0x004444); // Dark Yellow BGR
    }
};

#endif // GLASS_COMPARE_MANAGER_H
