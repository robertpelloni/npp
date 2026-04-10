#pragma once

#include <string>
#include <vector>
#include <chrono>

namespace Core {

// Deep comment: VersioningAutosave manages the "Never lose a file" requirement.
// Why: Standard autosave just overwrites the temp file. This system keeps a ledger
//      of all changes/saves over time.
// Side effects: High disk I/O if not managed correctly. We will store diffs/snapshots
//               in a structured local directory or SQLite DB.
// Implementation: Platform agnostic. Triggers on EventBus file-change events.

struct FileSnapshot {
    std::string filepath;
    std::string content;
    std::chrono::system_clock::time_point timestamp;
};

class VersioningAutosave {
public:
    VersioningAutosave();
    ~VersioningAutosave();

    // Triggered to save a new snapshot of a file
    void SaveSnapshot(const std::string& filepath, const std::string& content);

    // Retrieve all snapshots for a specific file
    std::vector<FileSnapshot> GetHistory(const std::string& filepath) const;

private:
    std::string m_storageDirectory;
};

} // namespace Core
