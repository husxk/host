#include "storage/file.h"

#include <system_error>
#include <utility>

#include "logger.h"

File::File(Path path, std::fstream stream)
    : path(std::move(path))
    , stream(std::move(stream))
{
}

std::optional<File> File::open(const Path &path, Mode mode)
{
    std::error_code                    ec;
    const std::filesystem::file_status status = std::filesystem::status(path, ec);

    if (std::filesystem::exists(status) && !std::filesystem::is_regular_file(status))
    {
        Logger::error("file: open %s: not a regular file", path.c_str());
        return std::nullopt;
    }

    std::ios::openmode flags = std::ios::binary | std::ios::in;

    if (mode == Mode::Append)
    {
        flags |= std::ios::app;
    }

    std::fstream stream(path, flags);

    if (!stream)
    {
        Logger::error("file: open %s failed", path.c_str());
        return std::nullopt;
    }

    return File(path, std::move(stream));
}

File::ReadResult File::read(std::uint64_t offset, char *buf, std::size_t max)
{
    this->stream.clear();
    this->stream.seekg(static_cast<std::streamoff>(offset));
    this->stream.read(buf, static_cast<std::streamsize>(max));

    if (this->stream.bad())
    {
        Logger::error("file: read %s failed", this->path.c_str());
        return {ReadStatus::Error, 0};
    }

    const std::size_t n   = static_cast<std::size_t>(this->stream.gcount());
    const bool        eof = this->stream.peek() == std::fstream::traits_type::eof();

    return {eof ? ReadStatus::Ok : ReadStatus::MoreData, n};
}

File::WriteResult File::append(const char *data, std::size_t n)
{
    this->stream.clear();
    this->stream.write(data, static_cast<std::streamsize>(n));
    this->stream.flush();

    if (!this->stream)
    {
        Logger::error("file: write %s failed", this->path.c_str());
        return {WriteStatus::Error, 0};
    }

    return {WriteStatus::Ok, n};
}
