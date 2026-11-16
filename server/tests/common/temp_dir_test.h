#pragma once

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

#include <gtest/gtest.h>

class TempDirTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        std::filesystem::remove_all(this->dir);
        std::filesystem::create_directories(this->dir);
    }

    void TearDown() override
    {
        std::filesystem::remove_all(this->dir);
    }

    std::filesystem::path path(const std::string &name) const
    {
        return this->dir / name;
    }

    std::filesystem::path make_file(const std::string &name, const std::string &content = "") const
    {
        const std::filesystem::path file = this->path(name);
        std::ofstream               stream(file, std::ios::binary);

        stream.write(content.data(), static_cast<std::streamsize>(content.size()));
        EXPECT_TRUE(stream) << "make_file " << file;

        return file;
    }

    std::string read_file(const std::string &name) const
    {
        std::ifstream stream(this->path(name), std::ios::binary);

        EXPECT_TRUE(stream) << "read_file " << this->path(name);

        return std::string(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
    }

    std::filesystem::path dir = TempDirTest::test_dir();

private:
    static std::filesystem::path test_dir()
    {
        const ::testing::TestInfo *info = ::testing::UnitTest::GetInstance()->current_test_info();

        return std::filesystem::temp_directory_path() / "host_tests"
               / (std::string(info->test_suite_name()) + "." + info->name());
    }
};
