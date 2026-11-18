#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "file_tree_common.h"

using Lines = std::vector<std::string>;

class FileTreeListTest : public FileTreeTest
{
};

TEST_F(FileTreeListTest, RootSortedWithDirSlash)
{
    this->make_file("root/b");
    this->make_file("root/a");
    this->make_dir("root/c");

    const std::optional<Lines> names = this->tree.list("");

    ASSERT_TRUE(names.has_value());
    EXPECT_EQ(*names, (Lines {"a", "b", "c/"}));
}

TEST_F(FileTreeListTest, RootAliasesListRoot)
{
    this->make_file("root/a");

    EXPECT_EQ(this->tree.list("/"), this->tree.list(""));
    EXPECT_EQ(this->tree.list("."), this->tree.list(""));
}

TEST_F(FileTreeListTest, SubdirectoryListsOneLevel)
{
    this->make_dir("root/a/sub");
    this->make_file("root/a/f");
    this->make_file("root/a/sub/deep");

    const std::optional<Lines> names = this->tree.list("a");

    ASSERT_TRUE(names.has_value());
    EXPECT_EQ(*names, (Lines {"f", "sub/"}));
}

TEST_F(FileTreeListTest, EmptyDirectoryIsEmpty)
{
    this->make_dir("root/empty");

    const std::optional<Lines> names = this->tree.list("empty");

    ASSERT_TRUE(names.has_value());
    EXPECT_TRUE(names->empty());
}

TEST_F(FileTreeListTest, InsideLinksListed)
{
    this->make_dir("root/a");
    this->make_file("root/f");
    std::filesystem::create_directory_symlink(this->path("root/a"), this->path("root/dir_link"));
    std::filesystem::create_symlink(this->path("root/f"), this->path("root/file_link"));

    const std::optional<Lines> names = this->tree.list("");

    ASSERT_TRUE(names.has_value());
    EXPECT_EQ(*names, (Lines {"a/", "dir_link/", "f", "file_link"}));
}

TEST_F(FileTreeListTest, OutsideAndDanglingLinksHidden)
{
    this->make_file("root/f");
    std::filesystem::create_directory_symlink(this->path("outside"), this->path("root/out_link"));
    std::filesystem::create_symlink(this->path("outside/o"), this->path("root/out_file_link"));
    std::filesystem::create_symlink("nowhere", this->path("root/dangling"));

    const std::optional<Lines> names = this->tree.list("");

    ASSERT_TRUE(names.has_value());
    EXPECT_EQ(*names, (Lines {"f"}));
}

TEST_F(FileTreeListTest, FileFails)
{
    this->make_file("root/f");

    EXPECT_FALSE(this->tree.list("f").has_value());
}

TEST_F(FileTreeListTest, MissingFails)
{
    EXPECT_FALSE(this->tree.list("missing").has_value());
}
