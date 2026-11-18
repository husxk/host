#include <filesystem>
#include <optional>

#include <gtest/gtest.h>

#include "file_tree_common.h"

class FileTreeDuTest : public FileTreeTest
{
};

TEST_F(FileTreeDuTest, FileReportsSize)
{
    this->make_file("root/f", "abc");

    const std::optional<UsageInfo> usage = this->tree.du("f");

    ASSERT_TRUE(usage.has_value());
    EXPECT_EQ(usage->size, 3u);
}

TEST_F(FileTreeDuTest, EmptyDirectoryIsZero)
{
    this->make_dir("root/a");

    const std::optional<UsageInfo> usage = this->tree.du("a");

    ASSERT_TRUE(usage.has_value());
    EXPECT_EQ(usage->size, 0u);
}

TEST_F(FileTreeDuTest, DirectorySumsRecursively)
{
    this->make_dir("root/a/sub");
    this->make_file("root/a/f", "abc");
    this->make_file("root/a/sub/g", "defgh");

    const std::optional<UsageInfo> usage = this->tree.du("a");

    ASSERT_TRUE(usage.has_value());
    EXPECT_EQ(usage->size, 8u);
}

TEST_F(FileTreeDuTest, RootSumsAll)
{
    this->make_dir("root/a");
    this->make_file("root/f", "abc");
    this->make_file("root/a/g", "defgh");

    const std::optional<UsageInfo> usage = this->tree.du("");

    ASSERT_TRUE(usage.has_value());
    EXPECT_EQ(usage->size, 8u);
}

TEST_F(FileTreeDuTest, SkipsLinksInside)
{
    this->make_dir("root/a");
    this->make_dir("root/b");
    this->make_file("root/a/f", "abc");
    this->make_file("root/b/g", "defgh");
    std::filesystem::create_symlink(this->path("root/a/f"), this->path("root/a/file_link"));
    std::filesystem::create_directory_symlink(this->path("root/b"), this->path("root/a/dir_link"));

    const std::optional<UsageInfo> usage = this->tree.du("a");

    ASSERT_TRUE(usage.has_value());
    EXPECT_EQ(usage->size, 3u);
}

TEST_F(FileTreeDuTest, LinkArgumentFollowed)
{
    this->make_dir("root/a");
    this->make_file("root/f", "abc");
    this->make_file("root/a/g", "defgh");
    std::filesystem::create_symlink(this->path("root/f"), this->path("root/file_link"));
    std::filesystem::create_directory_symlink(this->path("root/a"), this->path("root/dir_link"));

    const std::optional<UsageInfo> file_usage = this->tree.du("file_link");
    const std::optional<UsageInfo> dir_usage  = this->tree.du("dir_link");

    ASSERT_TRUE(file_usage.has_value());
    ASSERT_TRUE(dir_usage.has_value());
    EXPECT_EQ(file_usage->size, 3u);
    EXPECT_EQ(dir_usage->size, 5u);
}

TEST_F(FileTreeDuTest, MissingFails)
{
    EXPECT_FALSE(this->tree.du("missing").has_value());
}
