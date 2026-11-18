#include <filesystem>

#include <gtest/gtest.h>

#include "file_tree_common.h"

class FileTreeMkdirTest : public FileTreeTest
{
};

TEST_F(FileTreeMkdirTest, CreatesDirectory)
{
    EXPECT_TRUE(this->tree.mkdir("a"));
    EXPECT_TRUE(std::filesystem::is_directory(this->path("root/a")));
}

TEST_F(FileTreeMkdirTest, CreatesInSubdirectory)
{
    this->make_dir("root/a");

    EXPECT_TRUE(this->tree.mkdir("a/b"));
    EXPECT_TRUE(std::filesystem::is_directory(this->path("root/a/b")));
}

TEST_F(FileTreeMkdirTest, TrailingSlashCreates)
{
    EXPECT_TRUE(this->tree.mkdir("a/"));
    EXPECT_TRUE(std::filesystem::is_directory(this->path("root/a")));
}

TEST_F(FileTreeMkdirTest, ExistingDirectoryFails)
{
    this->make_dir("root/a");
    this->make_file("root/a/f", "abc");

    EXPECT_FALSE(this->tree.mkdir("a"));
    EXPECT_EQ(this->read_file("root/a/f"), "abc");
}

TEST_F(FileTreeMkdirTest, ExistingFileFails)
{
    this->make_file("root/f", "abc");

    EXPECT_FALSE(this->tree.mkdir("f"));
    EXPECT_EQ(this->read_file("root/f"), "abc");
}

TEST_F(FileTreeMkdirTest, MissingParentFails)
{
    EXPECT_FALSE(this->tree.mkdir("x/y"));
    EXPECT_FALSE(std::filesystem::exists(this->path("root/x")));
}

TEST_F(FileTreeMkdirTest, RootFails)
{
    EXPECT_FALSE(this->tree.mkdir(""));
}

TEST_F(FileTreeMkdirTest, InsideLinkCreatesInTarget)
{
    this->make_dir("root/a");
    std::filesystem::create_directory_symlink(this->path("root/a"), this->path("root/dir_link"));

    EXPECT_TRUE(this->tree.mkdir("dir_link/sub"));
    EXPECT_TRUE(std::filesystem::is_directory(this->path("root/a/sub")));
}

TEST_F(FileTreeMkdirTest, ExistingInsideLinkFails)
{
    this->make_dir("root/a");
    std::filesystem::create_directory_symlink(this->path("root/a"), this->path("root/dir_link"));

    EXPECT_FALSE(this->tree.mkdir("dir_link"));
    EXPECT_TRUE(std::filesystem::is_symlink(this->path("root/dir_link")));
}
