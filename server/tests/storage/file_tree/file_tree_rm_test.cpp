#include <filesystem>

#include <gtest/gtest.h>

#include "file_tree_common.h"

class FileTreeRmTest : public FileTreeTest
{
};

TEST_F(FileTreeRmTest, RemovesFile)
{
    this->make_file("root/f", "abc");

    EXPECT_TRUE(this->tree.rm("f"));
    EXPECT_FALSE(std::filesystem::exists(this->path("root/f")));
}

TEST_F(FileTreeRmTest, RemovesInSubdirectory)
{
    this->make_dir("root/a");
    this->make_file("root/a/f", "abc");

    EXPECT_TRUE(this->tree.rm("a/f"));
    EXPECT_FALSE(std::filesystem::exists(this->path("root/a/f")));
    EXPECT_TRUE(std::filesystem::is_directory(this->path("root/a")));
}

TEST_F(FileTreeRmTest, RemovesEmptyDirectory)
{
    this->make_dir("root/a");

    EXPECT_TRUE(this->tree.rm("a"));
    EXPECT_FALSE(std::filesystem::exists(this->path("root/a")));
}

TEST_F(FileTreeRmTest, TrailingSlashRemovesDirectory)
{
    this->make_dir("root/a");

    EXPECT_TRUE(this->tree.rm("a/"));
    EXPECT_FALSE(std::filesystem::exists(this->path("root/a")));
}

TEST_F(FileTreeRmTest, NonEmptyDirectoryFails)
{
    this->make_dir("root/a");
    this->make_file("root/a/f", "abc");

    EXPECT_FALSE(this->tree.rm("a"));
    EXPECT_EQ(this->read_file("root/a/f"), "abc");
}

TEST_F(FileTreeRmTest, MissingFails)
{
    EXPECT_FALSE(this->tree.rm("missing"));
}

TEST_F(FileTreeRmTest, RootFails)
{
    this->make_dir("root/a");

    for (const char *path : {"", "/", ".", "a/.."})
    {
        SCOPED_TRACE(path);
        EXPECT_FALSE(this->tree.rm(path));
    }

    EXPECT_TRUE(std::filesystem::is_directory(this->path("root/a")));
}

TEST_F(FileTreeRmTest, RemovesFileLinkNotTarget)
{
    this->make_file("root/f", "abc");
    std::filesystem::create_symlink(this->path("root/f"), this->path("root/file_link"));

    EXPECT_TRUE(this->tree.rm("file_link"));
    EXPECT_FALSE(std::filesystem::exists(std::filesystem::symlink_status(this->path("root/file_link"))));
    EXPECT_EQ(this->read_file("root/f"), "abc");
}

TEST_F(FileTreeRmTest, RemovesDirLinkNotTarget)
{
    this->make_dir("root/a");
    this->make_file("root/a/f", "abc");
    std::filesystem::create_directory_symlink(this->path("root/a"), this->path("root/dir_link"));

    EXPECT_TRUE(this->tree.rm("dir_link"));
    EXPECT_FALSE(std::filesystem::exists(std::filesystem::symlink_status(this->path("root/dir_link"))));
    EXPECT_EQ(this->read_file("root/a/f"), "abc");
}
