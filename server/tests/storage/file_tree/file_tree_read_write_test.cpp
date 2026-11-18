#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "file_tree_common.h"

class FileTreeReadWriteTest : public FileTreeTest
{
protected:
    static std::string pattern(std::size_t n)
    {
        std::string data(n, '\0');

        for (std::size_t i = 0; i < n; ++i)
        {
            data[i] = static_cast<char>(i % 251);
        }

        return data;
    }

    std::vector<char> buf = std::vector<char>(2 * FileTree::block_size);
};

TEST_F(FileTreeReadWriteTest, ReadReturnsContent)
{
    this->make_file("root/f", "abc");

    const FileTree::ReadResult result = this->tree.read("f", 0, this->buf.data(), this->buf.size());

    EXPECT_EQ(result.status, FileTree::ReadStatus::Ok);
    EXPECT_EQ(std::string(this->buf.data(), result.n), "abc");
}

TEST_F(FileTreeReadWriteTest, ReadCapsAtBlockSize)
{
    this->make_file("root/f", pattern(FileTree::block_size + 100));

    const FileTree::ReadResult result = this->tree.read("f", 0, this->buf.data(), this->buf.size());

    EXPECT_EQ(result.status, FileTree::ReadStatus::MoreData);
    EXPECT_EQ(result.n, FileTree::block_size);
}

TEST_F(FileTreeReadWriteTest, ReadHonoursSmallerMax)
{
    this->make_file("root/f", "abcdef");

    const FileTree::ReadResult result = this->tree.read("f", 0, this->buf.data(), 2);

    EXPECT_EQ(result.status, FileTree::ReadStatus::MoreData);
    EXPECT_EQ(std::string(this->buf.data(), result.n), "ab");
}

TEST_F(FileTreeReadWriteTest, ReadExactBlockSizeReturnsOk)
{
    this->make_file("root/f", pattern(FileTree::block_size));

    const FileTree::ReadResult result = this->tree.read("f", 0, this->buf.data(), this->buf.size());

    EXPECT_EQ(result.status, FileTree::ReadStatus::Ok);
    EXPECT_EQ(result.n, FileTree::block_size);
}

TEST_F(FileTreeReadWriteTest, ReadLargeFileInBlocks)
{
    constexpr std::size_t full_blocks = 5;

    const std::string content = pattern(full_blocks * FileTree::block_size + FileTree::block_size / 2);

    this->make_file("root/f", content);

    std::string          out;
    std::uint64_t        offset = 0;
    std::size_t          chunks = 0;
    FileTree::ReadResult result {};

    do
    {
        result = this->tree.read("f", offset, this->buf.data(), this->buf.size());
        out.append(this->buf.data(), result.n);
        offset += result.n;
        ++chunks;
    } while (result.status == FileTree::ReadStatus::MoreData && chunks <= full_blocks + 1);

    EXPECT_EQ(result.status, FileTree::ReadStatus::Ok);
    EXPECT_EQ(chunks, full_blocks + 1);
    EXPECT_EQ(out, content);
}

TEST_F(FileTreeReadWriteTest, ReadInsideLinkReadsTarget)
{
    this->make_file("root/f", "abc");
    std::filesystem::create_symlink(this->path("root/f"), this->path("root/file_link"));

    const FileTree::ReadResult result = this->tree.read("file_link", 0, this->buf.data(), this->buf.size());

    EXPECT_EQ(result.status, FileTree::ReadStatus::Ok);
    EXPECT_EQ(std::string(this->buf.data(), result.n), "abc");
}

TEST_F(FileTreeReadWriteTest, ReadDirectoryFails)
{
    this->make_dir("root/a");

    EXPECT_EQ(this->tree.read("a", 0, this->buf.data(), this->buf.size()).status, FileTree::ReadStatus::Error);
}

TEST_F(FileTreeReadWriteTest, ReadMissingFails)
{
    EXPECT_EQ(this->tree.read("missing", 0, this->buf.data(), this->buf.size()).status,
              FileTree::ReadStatus::Error);
}

TEST_F(FileTreeReadWriteTest, WriteCreatesFile)
{
    const FileTree::WriteResult result = this->tree.write("f", "abc", 3);

    EXPECT_EQ(result.status, FileTree::WriteStatus::Ok);
    EXPECT_EQ(result.n, 3u);
    EXPECT_EQ(this->read_file("root/f"), "abc");
}

TEST_F(FileTreeReadWriteTest, WriteAppendsToExisting)
{
    this->make_file("root/f", "abc");

    EXPECT_EQ(this->tree.write("f", "def", 3).status, FileTree::WriteStatus::Ok);
    EXPECT_EQ(this->read_file("root/f"), "abcdef");
}

TEST_F(FileTreeReadWriteTest, WriteInSubdirectory)
{
    this->make_dir("root/a");

    EXPECT_EQ(this->tree.write("a/f", "abc", 3).status, FileTree::WriteStatus::Ok);
    EXPECT_EQ(this->read_file("root/a/f"), "abc");
}

TEST_F(FileTreeReadWriteTest, WriteInsideLinkAppendsToTarget)
{
    this->make_file("root/f", "abc");
    std::filesystem::create_symlink(this->path("root/f"), this->path("root/file_link"));

    EXPECT_EQ(this->tree.write("file_link", "def", 3).status, FileTree::WriteStatus::Ok);
    EXPECT_EQ(this->read_file("root/f"), "abcdef");
}

TEST_F(FileTreeReadWriteTest, WriteMissingParentFails)
{
    EXPECT_EQ(this->tree.write("missing/f", "abc", 3).status, FileTree::WriteStatus::Error);
    EXPECT_FALSE(std::filesystem::exists(this->path("root/missing")));
}

TEST_F(FileTreeReadWriteTest, WriteDirectoryFails)
{
    this->make_dir("root/a");

    EXPECT_EQ(this->tree.write("a", "abc", 3).status, FileTree::WriteStatus::Error);
}

TEST_F(FileTreeReadWriteTest, WriteRootFails)
{
    EXPECT_EQ(this->tree.write("", "abc", 3).status, FileTree::WriteStatus::Error);
}

TEST_F(FileTreeReadWriteTest, WriteTrailingSlashFails)
{
    EXPECT_EQ(this->tree.write("f/", "abc", 3).status, FileTree::WriteStatus::Error);
    EXPECT_FALSE(std::filesystem::exists(this->path("root/f")));
}
