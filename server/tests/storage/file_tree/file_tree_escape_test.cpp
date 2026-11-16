#include <array>
#include <filesystem>
#include <iterator>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "file_tree_common.h"

class FileTreeEscapeTest : public FileTreeTest
{
protected:
    void SetUp() override
    {
        FileTreeTest::SetUp();

        this->make_dir("root/a");
        std::filesystem::create_directory_symlink(this->path("outside"), this->path("root/out_link"));
        std::filesystem::create_symlink("nowhere", this->path("root/dangling"));
    }

    std::vector<std::filesystem::path> escapes() const
    {
        return {
            "..",
            "../outside/o",
            "../outside/new",
            "a/../../outside/o",
            "out_link/o",
            "out_link",
            "dangling",
            this->path("outside/o"),
        };
    }

    void expect_outside_intact() const
    {
        const std::filesystem::directory_iterator it(this->path("outside"));

        EXPECT_EQ(std::distance(it, std::filesystem::directory_iterator {}), 1);
        EXPECT_EQ(this->read_file("outside/o"), "secret");
    }
};

TEST_F(FileTreeEscapeTest, StatRejectsEscapes)
{
    ASSERT_TRUE(this->tree.stat("a").has_value());

    for (const std::filesystem::path &path : this->escapes())
    {
        SCOPED_TRACE(path);
        EXPECT_FALSE(this->tree.stat(path).has_value());
    }
}

TEST_F(FileTreeEscapeTest, ListRejectsEscapes)
{
    ASSERT_TRUE(this->tree.list("a").has_value());

    for (const std::filesystem::path &path : this->escapes())
    {
        SCOPED_TRACE(path);
        EXPECT_FALSE(this->tree.list(path).has_value());
    }
}

TEST_F(FileTreeEscapeTest, ReadRejectsEscapes)
{
    this->make_file("root/a/f", "abc");

    std::array<char, 16> buf {};

    ASSERT_EQ(this->tree.read("a/f", 0, buf.data(), buf.size()).status, FileTree::ReadStatus::Ok);

    for (const std::filesystem::path &path : this->escapes())
    {
        SCOPED_TRACE(path);
        EXPECT_EQ(this->tree.read(path, 0, buf.data(), buf.size()).status, FileTree::ReadStatus::Error);
    }
}

TEST_F(FileTreeEscapeTest, WriteRejectsEscapes)
{
    const std::string data = "pwned";

    ASSERT_EQ(this->tree.write("a/f", data.data(), data.size()).status, FileTree::WriteStatus::Ok);

    for (const std::filesystem::path &path : this->escapes())
    {
        SCOPED_TRACE(path);
        EXPECT_EQ(this->tree.write(path, data.data(), data.size()).status, FileTree::WriteStatus::Error);
    }

    this->expect_outside_intact();
}

TEST_F(FileTreeEscapeTest, MkdirRejectsEscapes)
{
    ASSERT_TRUE(this->tree.mkdir("b"));

    for (const std::filesystem::path &path : this->escapes())
    {
        SCOPED_TRACE(path);
        EXPECT_FALSE(this->tree.mkdir(path));
    }

    this->expect_outside_intact();
}

TEST_F(FileTreeEscapeTest, RmRejectsEscapes)
{
    this->make_file("root/a/f");

    ASSERT_TRUE(this->tree.rm("a/f"));

    for (const std::filesystem::path &path : this->escapes())
    {
        if (path == "out_link" || path == "dangling")
        {
            continue;
        }

        SCOPED_TRACE(path);
        EXPECT_FALSE(this->tree.rm(path));
    }

    EXPECT_TRUE(this->tree.rm("out_link"));
    EXPECT_TRUE(this->tree.rm("dangling"));

    this->expect_outside_intact();
}

TEST_F(FileTreeEscapeTest, MvRejectsEscapes)
{
    this->make_file("root/a/f");

    ASSERT_TRUE(this->tree.mv("a/f", "a/g"));

    for (const std::filesystem::path &path : this->escapes())
    {
        SCOPED_TRACE(path);
        EXPECT_FALSE(this->tree.mv("a/g", path));
    }

    for (const std::filesystem::path &path : this->escapes())
    {
        if (path == "out_link" || path == "dangling")
        {
            continue;
        }

        SCOPED_TRACE(path);
        EXPECT_FALSE(this->tree.mv(path, "moved"));
    }

    EXPECT_TRUE(std::filesystem::exists(this->path("root/a/g")));
    EXPECT_TRUE(this->tree.mv("out_link", "moved_link"));
    EXPECT_TRUE(this->tree.mv("dangling", "moved_dangling"));

    this->expect_outside_intact();
}

TEST_F(FileTreeEscapeTest, DuRejectsEscapes)
{
    ASSERT_TRUE(this->tree.du("a").has_value());

    for (const std::filesystem::path &path : this->escapes())
    {
        SCOPED_TRACE(path);
        EXPECT_FALSE(this->tree.du(path).has_value());
    }
}
