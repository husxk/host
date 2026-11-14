#include <filesystem>

#include <gtest/gtest.h>

#include "file_common.h"
#include "storage/file.h"

class FileOpenTest : public FileTest
{
};

TEST_F(FileOpenTest, ExistingFileForReadSucceeds)
{
    const std::filesystem::path file = this->make_file("f", "abc");

    EXPECT_TRUE(File::open(file, File::Mode::Read).has_value());
}

TEST_F(FileOpenTest, MissingFileForReadFails)
{
    const std::filesystem::path file = this->path("missing");

    EXPECT_FALSE(File::open(file, File::Mode::Read).has_value());
    EXPECT_FALSE(std::filesystem::exists(file));
}

TEST_F(FileOpenTest, MissingFileForAppendCreatesIt)
{
    const std::filesystem::path file = this->path("new");

    EXPECT_TRUE(File::open(file, File::Mode::Append).has_value());
    EXPECT_TRUE(std::filesystem::is_regular_file(file));
}

TEST_F(FileOpenTest, MissingParentForAppendFails)
{
    const std::filesystem::path file = this->path("missing/new");

    EXPECT_FALSE(File::open(file, File::Mode::Append).has_value());
    EXPECT_FALSE(std::filesystem::exists(this->path("missing")));
}

TEST_F(FileOpenTest, DirectoryForReadFails)
{
    const std::filesystem::path sub = this->path("sub");

    std::filesystem::create_directory(sub);

    EXPECT_FALSE(File::open(sub, File::Mode::Read).has_value());
}

TEST_F(FileOpenTest, DirectoryForAppendFails)
{
    const std::filesystem::path sub = this->path("sub");

    std::filesystem::create_directory(sub);

    EXPECT_FALSE(File::open(sub, File::Mode::Append).has_value());
}

TEST_F(FileOpenTest, EmptyPathFails)
{
    EXPECT_FALSE(File::open("", File::Mode::Read).has_value());
    EXPECT_FALSE(File::open("", File::Mode::Append).has_value());
}

TEST_F(FileOpenTest, TrailingSlashOnFileFails)
{
    this->make_file("f", "abc");

    const std::filesystem::path file = this->path("f/");

    EXPECT_FALSE(File::open(file, File::Mode::Read).has_value());
    EXPECT_FALSE(File::open(file, File::Mode::Append).has_value());
}
