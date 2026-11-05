#pragma once

#include "doctest.h"

#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/DataHandler/TextFile.h"

#include <filesystem>
#include <random>
#include <string>
#include <system_error>

// A client folder of its own, for a test that writes or breaks the monster
// list. CTest runs the cases as parallel processes, so each gets a new name.
class TemporaryMonsterFolder
{
public:
    TemporaryMonsterFolder()
        : m_path(std::filesystem::temp_directory_path() / ("mu-monster-data-" + std::to_string(std::random_device{}())))
    {
        std::filesystem::create_directories(m_path / "Data" / "Monsters");
    }

    ~TemporaryMonsterFolder()
    {
        std::error_code error;
        std::filesystem::remove_all(m_path, error);
    }

    TemporaryMonsterFolder(const TemporaryMonsterFolder&) = delete;
    TemporaryMonsterFolder& operator=(const TemporaryMonsterFolder&) = delete;

    const std::filesystem::path& Path() const
    {
        return m_path;
    }

    std::filesystem::path MonstersFile() const
    {
        return m_path / Data::Monsters::GetMonstersFile();
    }

    void WriteMonsters(const std::string& text) const
    {
        std::string error;
        REQUIRE(Data::WriteTextFile(MonstersFile(), text, error));
    }

private:
    std::filesystem::path m_path;
};
