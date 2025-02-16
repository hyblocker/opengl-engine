#pragma once

#include "engine/renderer/scene_graph.hpp"
#include "engine/renderer/ui_components.hpp"

#include <algorithm>

constexpr uint32_t k_MAX_LEADERBOARD_ENTRIES = 5;
constexpr int32_t k_INVALID_SCORE = -1;

class Leaderboard {

public:
    struct LeaderboardEntry {
        std::string initials="\0\0\0";
        int32_t score = k_INVALID_SCORE;

        inline const bool isValid() const { return score >= 0; }
    };

    void addEntry(const std::string& initials, int32_t score);

    // for iterating
    const inline size_t size() const { return m_numEntries; }
    const LeaderboardEntry& getEntry(size_t index) const;

    bool save(const std::string& path);
    bool load(const std::string& path);

private:
    LeaderboardEntry m_entries[k_MAX_LEADERBOARD_ENTRIES + 1] = {}; // add extra entry as buffer
    size_t m_numEntries = 0;
};