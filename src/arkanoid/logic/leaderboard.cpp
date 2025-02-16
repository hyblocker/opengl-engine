#include "leaderboard.hpp"
#include "engine/log.hpp"

void Leaderboard::addEntry(const std::string& initials, int32_t score) {
    if (initials.empty()) {
        return;
    }

    LeaderboardEntry newEntry;
    newEntry.initials = initials;
    newEntry.score = score;

    bool mustPurgeEntry = false;

    // if less entries then total, insert at back
    if (m_numEntries < k_MAX_LEADERBOARD_ENTRIES) {
        m_entries[m_numEntries] = newEntry;
        m_numEntries++;
    } else {
        m_entries[k_MAX_LEADERBOARD_ENTRIES] = newEntry;
        mustPurgeEntry = true;
    }

    std::sort(m_entries, m_entries + k_MAX_LEADERBOARD_ENTRIES + 1, [](const LeaderboardEntry& a, const LeaderboardEntry& b) {
        return a.score > b.score;
    });

    if (mustPurgeEntry) {
        m_entries[k_MAX_LEADERBOARD_ENTRIES].initials = "";
        m_entries[k_MAX_LEADERBOARD_ENTRIES].score = k_INVALID_SCORE; // mark as invalid
    }
}

const Leaderboard::LeaderboardEntry& Leaderboard::getEntry(size_t index) const {
    if (index < m_numEntries) {
        return m_entries[index];
    }
    else {
        return { .score = k_INVALID_SCORE };
    }
}

bool Leaderboard::save(const std::string& path) {
    FILE* file = fopen(path.c_str(), "wb");

    if (!file) {
        LOG_WARN("Failed to open leaderboard file {}! Aborting...", path);
        return false;
    }

    // numEntries
    // [{initials,score}...]

    fwrite(&m_numEntries, sizeof(m_numEntries), 1, file);
    for (size_t i = 0; i < m_numEntries; ++i) {
        const auto& entry = m_entries[i];

        // initials are 3 chars
        fwrite(entry.initials.c_str(), 1, 3, file);
        fwrite(&entry.score, sizeof(entry.score), 1, file);
    }

    fclose(file);
    return true;
}

bool Leaderboard::load(const std::string& path) {
    FILE* file = fopen(path.c_str(), "rb");

    if (!file) {
        return false;
    }

    if (fread(&m_numEntries, sizeof(m_numEntries), 1, file) != 1) {
        LOG_WARN("Failed to read m_numEntries from {}! Aborting...", path);
        fclose(file);
        return false;
    }

    m_numEntries = std::min(m_numEntries, (size_t)(k_MAX_LEADERBOARD_ENTRIES));

    for (size_t i = 0; i < m_numEntries; ++i) {
        LeaderboardEntry entry;

        char initials[4] = {}; // 3 chars + null terminator
        if (fread(initials, 1, 3, file) != 3) {
            LOG_WARN("Failed to read m_entries[{}].initials from {}! Aborting...", i, path);
            fclose(file);
            return false;
        }
        entry.initials = initials;

        if (fread(&entry.score, sizeof(entry.score), 1, file) != 1) {
            LOG_WARN("Failed to read m_entries[{}].score from {}! Aborting...", i, path);
            fclose(file);
            return false;
        }

        m_entries[i] = entry;
    }

    fclose(file);
    return false;
}