#include <filesystem>
#include <memory>
#include <string>
#include <variant>

#include <gtest/gtest.h>

#include "command/command_dispatcher.h"
#include "common/temp_dir_test.h"
#include "storage/file_tree.h"

using Status  = CommandDispatcher::Status;
using Result  = CommandDispatcher::Result;
using Strings = CommandDispatcher::Strings;
using RawData = CommandDispatcher::RawData;

class CommandDispatcherTest : public TempDirTest
{
protected:
    CommandDispatcher dispatcher {std::make_shared<FileTree>(this->dir)};
};

TEST_F(CommandDispatcherTest, LsReturnsNames)
{
    this->make_file("f");
    std::filesystem::create_directory(this->path("a"));

    const Result result = this->dispatcher.dispatch(LsCommand {""});

    ASSERT_EQ(result.status, Status::Ok);
    ASSERT_TRUE(std::holds_alternative<Strings>(result.body));
    EXPECT_EQ(std::get<Strings>(result.body), (Strings {"a/", "f"}));
}

TEST_F(CommandDispatcherTest, LsMissingFails)
{
    const Result result = this->dispatcher.dispatch(LsCommand {"missing"});

    EXPECT_EQ(result.status, Status::Failed);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(result.body));
}

TEST_F(CommandDispatcherTest, ShowReturnsData)
{
    this->make_file("f", "abc");

    const Result result = this->dispatcher.dispatch(ShowCommand {"f"});

    ASSERT_EQ(result.status, Status::Ok);
    ASSERT_TRUE(std::holds_alternative<RawData>(result.body));
    EXPECT_EQ(std::get<RawData>(result.body), (RawData {'a', 'b', 'c'}));
}

TEST_F(CommandDispatcherTest, ShowLargeFileReturnsMoreData)
{
    this->make_file("f", std::string(FileTree::block_size + 1, 'x'));

    const Result result = this->dispatcher.dispatch(ShowCommand {"f"});

    ASSERT_EQ(result.status, Status::MoreData);
    ASSERT_TRUE(std::holds_alternative<RawData>(result.body));
    EXPECT_EQ(std::get<RawData>(result.body).size(), FileTree::block_size);
}

TEST_F(CommandDispatcherTest, ShowFromOffset)
{
    this->make_file("f", "abcdef");

    const Result result = this->dispatcher.dispatch(ShowCommand {"f", 2});

    ASSERT_EQ(result.status, Status::Ok);
    ASSERT_TRUE(std::holds_alternative<RawData>(result.body));
    EXPECT_EQ(std::get<RawData>(result.body), (RawData {'c', 'd', 'e', 'f'}));
}

TEST_F(CommandDispatcherTest, ShowMissingFails)
{
    const Result result = this->dispatcher.dispatch(ShowCommand {"missing"});

    EXPECT_EQ(result.status, Status::Failed);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(result.body));
}

TEST_F(CommandDispatcherTest, WriteAppendsData)
{
    this->make_file("f", "abc");

    const Result result = this->dispatcher.dispatch(WriteCommand {"f", {'d', 'e', 'f'}});

    EXPECT_EQ(result.status, Status::Ok);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(result.body));
    EXPECT_EQ(this->read_file("f"), "abcdef");
}

TEST_F(CommandDispatcherTest, WriteDirectoryFails)
{
    std::filesystem::create_directory(this->path("a"));

    const Result result = this->dispatcher.dispatch(WriteCommand {"a", {'a'}});

    EXPECT_EQ(result.status, Status::Failed);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(result.body));
}

TEST_F(CommandDispatcherTest, StatReturnsInfo)
{
    this->make_file("f", "abc");

    const Result result = this->dispatcher.dispatch(StatCommand {"f"});

    ASSERT_EQ(result.status, Status::Ok);
    ASSERT_TRUE(std::holds_alternative<FileInfo>(result.body));
    EXPECT_EQ(std::get<FileInfo>(result.body).type, FileInfo::Type::File);
    EXPECT_EQ(std::get<FileInfo>(result.body).size, 3u);
}

TEST_F(CommandDispatcherTest, StatMissingFails)
{
    const Result result = this->dispatcher.dispatch(StatCommand {"missing"});

    EXPECT_EQ(result.status, Status::Failed);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(result.body));
}

TEST_F(CommandDispatcherTest, MkdirCreatesDirectory)
{
    const Result result = this->dispatcher.dispatch(MkdirCommand {"a"});

    EXPECT_EQ(result.status, Status::Ok);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(result.body));
    EXPECT_TRUE(std::filesystem::is_directory(this->path("a")));
}

TEST_F(CommandDispatcherTest, MkdirExistingFails)
{
    std::filesystem::create_directory(this->path("a"));

    const Result result = this->dispatcher.dispatch(MkdirCommand {"a"});

    EXPECT_EQ(result.status, Status::Failed);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(result.body));
}

TEST_F(CommandDispatcherTest, MvRenamesFile)
{
    this->make_file("f", "abc");

    const Result result = this->dispatcher.dispatch(MvCommand {"f", "g"});

    EXPECT_EQ(result.status, Status::Ok);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(result.body));
    EXPECT_EQ(this->read_file("g"), "abc");
}

TEST_F(CommandDispatcherTest, MvMissingFails)
{
    const Result result = this->dispatcher.dispatch(MvCommand {"missing", "g"});

    EXPECT_EQ(result.status, Status::Failed);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(result.body));
}

TEST_F(CommandDispatcherTest, RmRemovesFile)
{
    this->make_file("f");

    const Result result = this->dispatcher.dispatch(RmCommand {"f"});

    EXPECT_EQ(result.status, Status::Ok);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(result.body));
    EXPECT_FALSE(std::filesystem::exists(this->path("f")));
}

TEST_F(CommandDispatcherTest, RmMissingFails)
{
    const Result result = this->dispatcher.dispatch(RmCommand {"missing"});

    EXPECT_EQ(result.status, Status::Failed);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(result.body));
}

TEST_F(CommandDispatcherTest, DuReturnsUsage)
{
    this->make_file("f", "abc");

    const Result result = this->dispatcher.dispatch(DuCommand {"f"});

    ASSERT_EQ(result.status, Status::Ok);
    ASSERT_TRUE(std::holds_alternative<UsageInfo>(result.body));
    EXPECT_EQ(std::get<UsageInfo>(result.body).size, 3u);
}

TEST_F(CommandDispatcherTest, DuMissingFails)
{
    const Result result = this->dispatcher.dispatch(DuCommand {"missing"});

    EXPECT_EQ(result.status, Status::Failed);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(result.body));
}

TEST_F(CommandDispatcherTest, ExitReturnsExit)
{
    const Result result = this->dispatcher.dispatch(ExitCommand {});

    EXPECT_EQ(result.status, Status::Exit);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(result.body));
}
