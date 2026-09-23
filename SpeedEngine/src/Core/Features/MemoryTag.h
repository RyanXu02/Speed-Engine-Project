#pragma once

namespace SE
{
    struct MemoryTag
    {
        std::string category;
        std::string item;
    };

    struct MemoryTagHash
    {
        std::size_t operator()(const MemoryTag& tag) const
        {
            std::size_t categoryHash = std::hash<std::string>{}(tag.category);
            std::size_t itemHash = std::hash<std::string>{}(tag.item);
            return categoryHash ^ (itemHash << 1);
        }

        bool operator()(const MemoryTag& a, const MemoryTag& b) const
        {
            return a.category == b.category && a.item == b.item;
        }
    };

    inline const MemoryTag& tagStorage(std::string category, std::string item = "Unknown")
    {
        static std::unordered_map<MemoryTag, MemoryTag, MemoryTagHash, MemoryTagHash> tags;
        static std::mutex mutex;
        std::lock_guard lock(mutex);
        MemoryTag key{ category, item };
        return tags.emplace(key, key).first->second;
    }
} // namespace SE
