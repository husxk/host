#include <filesystem>

#include <gtest/gtest.h>

#include "file_tree_common.h"

class FileTreeMvTest : public FileTreeTest
{
};

TEST_F(FileTreeMvTest, RenamesFile)
{
    this->make_file("root/f", "abc");

    EXPECT_TRUE(this->tree.mv("f", "g"));
    EXPECT_EQ(this->read_file("root/g"), "abc");
    EXPECT_FALSE(std::filesystem::exists(this->path("root/f")));
}

TEST_F(FileTreeMvTest, RenamesDirectory)
{
    this->make_dir("root/a");
    this->make_file("root/a/f", "abc");

    EXPECT_TRUE(this->tree.mv("a", "b"));
    EXPECT_EQ(this->read_file("root/b/f"), "abc");
    EXPECT_FALSE(std::filesystem::exists(this->path("root/a")));
}

TEST_F(FileTreeMvTest, IntoExistingDirectory)
{
    this->make_dir("root/a");
    this->make_file("root/f", "abc");

    EXPECT_TRUE(this->tree.mv("f", "a"));
    EXPECT_EQ(this->read_file("root/a/f"), "abc");
    EXPECT_FALSE(std::filesystem::exists(this->path("root/f")));
}

TEST_F(FileTreeMvTest, ExistingFileDestinationFails)
{
    this->make_file("root/f", "abc");
    this->make_file("root/g", "def");

    EXPECT_FALSE(this->tree.mv("f", "g"));
    EXPECT_EQ(this->read_file("root/f"), "abc");
    EXPECT_EQ(this->read_file("root/g"), "def");
}

TEST_F(FileTreeMvTest, SameNameInDirectoryFails)
{
    this->make_dir("root/a");
    this->make_file("root/a/f", "def");
    this->make_file("root/f", "abc");

    EXPECT_FALSE(this->tree.mv("f", "a"));
    EXPECT_EQ(this->read_file("root/f"), "abc");
    EXPECT_EQ(this->read_file("root/a/f"), "def");
}

TEST_F(FileTreeMvTest, SameSourceAndDestinationFails)
{
    this->make_file("root/f", "abc");

    EXPECT_FALSE(this->tree.mv("f", "f"));
    EXPECT_EQ(this->read_file("root/f"), "abc");
}

TEST_F(FileTreeMvTest, MissingSourceFails)
{
    EXPECT_FALSE(this->tree.mv("missing", "g"));
    EXPECT_FALSE(std::filesystem::exists(this->path("root/g")));
}

TEST_F(FileTreeMvTest, MissingDestinationParentFails)
{
    this->make_file("root/f", "abc");

    EXPECT_FALSE(this->tree.mv("f", "x/g"));
    EXPECT_EQ(this->read_file("root/f"), "abc");
    EXPECT_FALSE(std::filesystem::exists(this->path("root/x")));
}

TEST_F(FileTreeMvTest, DirectoryIntoItselfFails)
{
    this->make_dir("root/a");
    this->make_file("root/a/f", "abc");

    EXPECT_FALSE(this->tree.mv("a", "a/b"));
    EXPECT_EQ(this->read_file("root/a/f"), "abc");
    EXPECT_FALSE(std::filesystem::exists(this->path("root/a/b")));
}

TEST_F(FileTreeMvTest, RootSourceFails)
{
    EXPECT_FALSE(this->tree.mv("", "x"));
    EXPECT_FALSE(std::filesystem::exists(this->path("root/x")));
}

TEST_F(FileTreeMvTest, MovesLinkNotTarget)
{
    this->make_file("root/f", "abc");
    std::filesystem::create_symlink(this->path("root/f"), this->path("root/file_link"));

    EXPECT_TRUE(this->tree.mv("file_link", "moved"));
    EXPECT_TRUE(std::filesystem::is_symlink(this->path("root/moved")));
    EXPECT_FALSE(std::filesystem::exists(std::filesystem::symlink_status(this->path("root/file_link"))));
    EXPECT_EQ(this->read_file("root/f"), "abc");
}

TEST_F(FileTreeMvTest, IntoInsideDirLink)
{
    this->make_dir("root/a");
    this->make_file("root/f", "abc");
    std::filesystem::create_directory_symlink(this->path("root/a"), this->path("root/dir_link"));

    EXPECT_TRUE(this->tree.mv("f", "dir_link"));
    EXPECT_EQ(this->read_file("root/a/f"), "abc");
    EXPECT_TRUE(std::filesystem::is_symlink(this->path("root/dir_link")));
}
