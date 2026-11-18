#include <chrono>
#include <filesystem>
#include <optional>

#include <gtest/gtest.h>

#include "file_tree_common.h"

class FileTreeStatTest : public FileTreeTest
{
};

TEST_F(FileTreeStatTest, FileReportsTypeAndSize)
{
    this->make_file("root/f", "abc");

    const std::optional<FileInfo> info = this->tree.stat("f");

    ASSERT_TRUE(info.has_value());
    EXPECT_EQ(info->type, FileInfo::Type::File);
    EXPECT_EQ(info->size, 3u);
}

TEST_F(FileTreeStatTest, EmptyFileReportsSizeZero)
{
    this->make_file("root/f");

    const std::optional<FileInfo> info = this->tree.stat("f");

    ASSERT_TRUE(info.has_value());
    EXPECT_EQ(info->type, FileInfo::Type::File);
    EXPECT_EQ(info->size, 0u);
}

TEST_F(FileTreeStatTest, FileReportsMtime)
{
    using namespace std::chrono;

    const sys_seconds mtime = sys_days {year {2020} / 1 / 2} + hours {3} + minutes {4} + seconds {5};

    std::filesystem::last_write_time(this->make_file("root/f"), clock_cast<file_clock>(mtime));

    const std::optional<FileInfo> info = this->tree.stat("f");

    ASSERT_TRUE(info.has_value());
    EXPECT_EQ(floor<seconds>(info->mtime), mtime);
}

TEST_F(FileTreeStatTest, DirectoryReportsSizeZero)
{
    this->make_dir("root/a");

    const std::optional<FileInfo> info = this->tree.stat("a");

    ASSERT_TRUE(info.has_value());
    EXPECT_EQ(info->type, FileInfo::Type::Directory);
    EXPECT_EQ(info->size, 0u);
}

TEST_F(FileTreeStatTest, RootIsDirectory)
{
    const std::optional<FileInfo> info = this->tree.stat("");

    ASSERT_TRUE(info.has_value());
    EXPECT_EQ(info->type, FileInfo::Type::Directory);
}

TEST_F(FileTreeStatTest, InsideDirLinkReportsTarget)
{
    this->make_dir("root/a");
    std::filesystem::create_directory_symlink(this->path("root/a"), this->path("root/dir_link"));

    const std::optional<FileInfo> info = this->tree.stat("dir_link");

    ASSERT_TRUE(info.has_value());
    EXPECT_EQ(info->type, FileInfo::Type::Directory);
}

TEST_F(FileTreeStatTest, InsideFileLinkReportsTarget)
{
    this->make_file("root/f", "abc");
    std::filesystem::create_symlink(this->path("root/f"), this->path("root/file_link"));

    const std::optional<FileInfo> info = this->tree.stat("file_link");

    ASSERT_TRUE(info.has_value());
    EXPECT_EQ(info->type, FileInfo::Type::File);
    EXPECT_EQ(info->size, 3u);
}

TEST_F(FileTreeStatTest, MissingFails)
{
    EXPECT_FALSE(this->tree.stat("missing").has_value());
}
