#pragma once

#include "Core/Globals/_enum.h"
#include "Render/Models/ZzzBMD.h"

#include <memory>

// The game's model slots for a test: Models points to empty slots of the test
// until the end of the test; the models the test opened are released then.
class TestModelSlots
{
public:
    TestModelSlots() : m_slots(new BMD[MAX_MODELS]), m_previous(Models)
    {
        Models = m_slots.get();
    }

    ~TestModelSlots()
    {
        Models = m_previous;
    }

    TestModelSlots(const TestModelSlots&) = delete;
    TestModelSlots& operator=(const TestModelSlots&) = delete;

private:
    std::unique_ptr<BMD[]> m_slots;
    BMD* m_previous;
};
