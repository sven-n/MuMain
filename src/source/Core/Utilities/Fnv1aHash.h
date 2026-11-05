#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

// FNV-1a, 64-bit, over the bytes of plain values: a fingerprint of state, to tell whether something changed it. Not a
// security hash. Add members one by one; a struct's padding bytes would make the fingerprint noise.
class Fnv1aHash
{
public:
    void Add(const void* data, std::size_t size)
    {
        const auto* bytes = static_cast<const unsigned char*>(data);
        for (std::size_t i = 0; i < size; ++i)
        {
            m_value ^= bytes[i];
            m_value *= 1099511628211ull;
        }
    }

    template <typename T>
        requires std::is_arithmetic_v<T> || std::is_enum_v<T> || std::is_pointer_v<T>
    void Add(T value)
    {
        Add(&value, sizeof(value));
    }

    [[nodiscard]] std::uint64_t Value() const
    {
        return m_value;
    }

private:
    std::uint64_t m_value = 1469598103934665603ull;
};
