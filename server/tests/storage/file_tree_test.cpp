#include <filesystem>
#include <optional>

#include <gtest/gtest.h>

#include "storage/file_tree.h"

TEST(FileTree, MkdirThenStat)
{
    const std::filesystem::path root = std::filesystem::temp_directory_path() / "host_tests_file_tree";

    std::filesystem::remove_all(root);
    std::filesystem::create_directory(root);

    FileTree tree(root);

    ASSERT_TRUE(tree.mkdir("a"));

    const std::optional<FileInfo> info = tree.stat("a");

    ASSERT_TRUE(info.has_value());
    EXPECT_EQ(info->type, FileInfo::Type::Directory);

    std::filesystem::remove_all(root);
}
