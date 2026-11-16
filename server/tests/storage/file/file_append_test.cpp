#include <optional>

#include <gtest/gtest.h>

#include "file_common.h"
#include "storage/file.h"

class FileAppendTest : public FileTest
{
};

TEST_F(FileAppendTest, ToNewFile)
{
    std::optional<File> file = File::open(this->path("f"), File::Mode::Append);

    ASSERT_TRUE(file.has_value());

    const File::WriteResult result = file->append("abc", 3);

    EXPECT_EQ(result.status, File::WriteStatus::Ok);
    EXPECT_EQ(result.n, 3u);
    EXPECT_EQ(this->read_file("f"), "abc");
}

TEST_F(FileAppendTest, TwiceConcatenates)
{
    std::optional<File> file = File::open(this->path("f"), File::Mode::Append);

    ASSERT_TRUE(file.has_value());

    EXPECT_EQ(file->append("abc", 3).status, File::WriteStatus::Ok);
    EXPECT_EQ(file->append("def", 3).status, File::WriteStatus::Ok);
    EXPECT_EQ(this->read_file("f"), "abcdef");
}

TEST_F(FileAppendTest, KeepsExistingContent)
{
    std::optional<File> file = File::open(this->make_file("f", "abc"), File::Mode::Append);

    ASSERT_TRUE(file.has_value());

    EXPECT_EQ(file->append("def", 3).status, File::WriteStatus::Ok);
    EXPECT_EQ(this->read_file("f"), "abcdef");
}

TEST_F(FileAppendTest, ZeroBytesIsNoop)
{
    std::optional<File> file = File::open(this->make_file("f", "abc"), File::Mode::Append);

    ASSERT_TRUE(file.has_value());

    const File::WriteResult result = file->append("", 0);

    EXPECT_EQ(result.status, File::WriteStatus::Ok);
    EXPECT_EQ(result.n, 0u);
    EXPECT_EQ(this->read_file("f"), "abc");
}

TEST_F(FileAppendTest, ReadModeFails)
{
    std::optional<File> file = File::open(this->make_file("f", "abc"), File::Mode::Read);

    ASSERT_TRUE(file.has_value());

    const File::WriteResult result = file->append("def", 3);

    EXPECT_EQ(result.status, File::WriteStatus::Error);
    EXPECT_EQ(result.n, 0u);
    EXPECT_EQ(this->read_file("f"), "abc");
}
