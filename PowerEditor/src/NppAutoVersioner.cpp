#include "NppAutoVersioner.h"

// Deep comment: Implementation of NppAutoVersioner
// Why: Checks if a modification warrants a snapshot (e.g. SCN_SAVEPOINTREACHED).
//      Currently a skeleton that filters events to avoid blocking the hot loop.

void NppAutoVersioner::handleModification(SCNotification* notification) {
    if (!notification) return;

    // Fast return: Only care about specific modification types, like a save point being reached
    if (notification->nmhdr.code != SCN_SAVEPOINTREACHED && notification->nmhdr.code != SCN_MODIFIED) {
        return;
    }

    // In a future implementation, this is where we'd dispatch an asynchronous
    // event via EventBus to the Go backend `VersionHistoryManager`.
    // We avoid string copies and deep comparisons here.
}
