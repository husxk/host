#pragma once

#include <filesystem>
#include <string>

#include "common/temp_dir_test.h"
#include "storage/file_tree.h"

class FileTreeTest : public TempDirTest
{
protected:
    void SetUp() override
    {
        TempDirTest::SetUp();

        this->make_dir("root");
        this->make_dir("outside");
        this->make_file("outside/o", "secret");
    }

    std::filesystem::path make_dir(const std::string &name) const
    {
        const std::filesystem::path sub = this->path(name);

        std::filesystem::create_directories(sub);

        return sub;
    }

    std::filesystem::path root = this->path("root");
    FileTree              tree {this->root};
};
