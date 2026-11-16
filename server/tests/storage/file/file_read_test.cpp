#include <array>
#include <cstdint>
#include <optional>
#include <string>

#include <gtest/gtest.h>

#include "file_common.h"
#include "storage/file.h"

class FileReadTest : public FileTest
{
};

TEST_F(FileReadTest, WholeFileReturnsOk)
{
    std::optional<File> file = File::open(this->make_file("f", "abc"), File::Mode::Read);

    ASSERT_TRUE(file.has_value());

    std::array<char, 10>   buf {};
    const File::ReadResult result = file->read(0, buf.data(), buf.size());

    EXPECT_EQ(result.status, File::ReadStatus::Ok);
    EXPECT_EQ(result.n, 3u);
    EXPECT_EQ(std::string(buf.data(), result.n), "abc");
}

TEST_F(FileReadTest, PartialReturnsMoreData)
{
    std::optional<File> file = File::open(this->make_file("f", "abcdef"), File::Mode::Read);

    ASSERT_TRUE(file.has_value());

    std::array<char, 3>    buf {};
    const File::ReadResult result = file->read(0, buf.data(), buf.size());

    EXPECT_EQ(result.status, File::ReadStatus::MoreData);
    EXPECT_EQ(result.n, 3u);
    EXPECT_EQ(std::string(buf.data(), result.n), "abc");
}

TEST_F(FileReadTest, ExactRemainingReturnsOk)
{
    std::optional<File> file = File::open(this->make_file("f", "abc"), File::Mode::Read);

    ASSERT_TRUE(file.has_value());

    std::array<char, 3>    buf {};
    const File::ReadResult result = file->read(0, buf.data(), buf.size());

    EXPECT_EQ(result.status, File::ReadStatus::Ok);
    EXPECT_EQ(result.n, 3u);
    EXPECT_EQ(std::string(buf.data(), result.n), "abc");
}

TEST_F(FileReadTest, FromOffset)
{
    std::optional<File> file = File::open(this->make_file("f", "abcdef"), File::Mode::Read);

    ASSERT_TRUE(file.has_value());

    std::array<char, 10>   buf {};
    const File::ReadResult result = file->read(2, buf.data(), buf.size());

    EXPECT_EQ(result.status, File::ReadStatus::Ok);
    EXPECT_EQ(result.n, 4u);
    EXPECT_EQ(std::string(buf.data(), result.n), "cdef");
}

TEST_F(FileReadTest, ChunksReassembleFile)
{
    std::optional<File> file = File::open(this->make_file("f", "0123456789"), File::Mode::Read);

    ASSERT_TRUE(file.has_value());

    std::string      out;
    std::uint64_t    offset = 0;
    int              chunks = 0;
    File::ReadResult result {};

    do
    {
        std::array<char, 3> buf {};

        result = file->read(offset, buf.data(), buf.size());
        out.append(buf.data(), result.n);
        offset += result.n;
        ++chunks;
    } while (result.status == File::ReadStatus::MoreData && chunks < 10);

    EXPECT_EQ(result.status, File::ReadStatus::Ok);
    EXPECT_EQ(chunks, 4);
    EXPECT_EQ(out, "0123456789");
}

TEST_F(FileReadTest, AtEofReturnsOkEmpty)
{
    std::optional<File> file = File::open(this->make_file("f", "abc"), File::Mode::Read);

    ASSERT_TRUE(file.has_value());

    std::array<char, 10>   buf {};
    const File::ReadResult result = file->read(3, buf.data(), buf.size());

    EXPECT_EQ(result.status, File::ReadStatus::Ok);
    EXPECT_EQ(result.n, 0u);
}

TEST_F(FileReadTest, PastEofReturnsOkEmpty)
{
    std::optional<File> file = File::open(this->make_file("f", "abc"), File::Mode::Read);

    ASSERT_TRUE(file.has_value());

    std::array<char, 10>   buf {};
    const File::ReadResult result = file->read(100, buf.data(), buf.size());

    EXPECT_EQ(result.status, File::ReadStatus::Ok);
    EXPECT_EQ(result.n, 0u);
}

TEST_F(FileReadTest, EmptyFileReturnsOkEmpty)
{
    std::optional<File> file = File::open(this->make_file("f"), File::Mode::Read);

    ASSERT_TRUE(file.has_value());

    std::array<char, 10>   buf {};
    const File::ReadResult result = file->read(0, buf.data(), buf.size());

    EXPECT_EQ(result.status, File::ReadStatus::Ok);
    EXPECT_EQ(result.n, 0u);
}

TEST_F(FileReadTest, BinaryBytesPreserved)
{
    const std::string   content("a\0b\n\xff", 5);
    std::optional<File> file = File::open(this->make_file("f", content), File::Mode::Read);

    ASSERT_TRUE(file.has_value());

    std::array<char, 10>   buf {};
    const File::ReadResult result = file->read(0, buf.data(), buf.size());

    EXPECT_EQ(result.status, File::ReadStatus::Ok);
    EXPECT_EQ(result.n, 5u);
    EXPECT_EQ(std::string(buf.data(), result.n), content);
}

TEST_F(FileReadTest, ReuseAfterEof)
{
    std::optional<File> file = File::open(this->make_file("f", "abc"), File::Mode::Read);

    ASSERT_TRUE(file.has_value());

    std::array<char, 10> buf {};

    ASSERT_EQ(file->read(0, buf.data(), buf.size()).status, File::ReadStatus::Ok);

    buf = {};

    const File::ReadResult result = file->read(0, buf.data(), buf.size());

    EXPECT_EQ(result.status, File::ReadStatus::Ok);
    EXPECT_EQ(result.n, 3u);
    EXPECT_EQ(std::string(buf.data(), result.n), "abc");
}

TEST_F(FileReadTest, ZeroMaxReturnsMoreData)
{
    std::optional<File> file = File::open(this->make_file("f", "abc"), File::Mode::Read);

    ASSERT_TRUE(file.has_value());

    std::array<char, 10>   buf {};
    const File::ReadResult result = file->read(0, buf.data(), 0);

    EXPECT_EQ(result.status, File::ReadStatus::MoreData);
    EXPECT_EQ(result.n, 0u);
}
