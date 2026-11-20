#include "logger.h"

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <string>

#include <fcntl.h>
#include <unistd.h>

Logger &Logger::instance()
{
    static Logger logger;
    return logger;
}

std::filesystem::path Logger::default_log_path() const
{
    if (const char *home = std::getenv("HOME"))
    {
        return std::filesystem::path(home) / log_file_name;
    }

    return std::filesystem::current_path() / log_file_name;
}

const char *Logger::level_label(Level level) const
{
    switch (level)
    {
    case Level::Info:
        return "info";
    case Level::Warning:
        return "warning";
    case Level::Error:
        return "error";
    }

    return "info";
}

bool Logger::redirect_stdio(const std::filesystem::path &path)
{
    const std::string log_path = path.string();

    const int open_flags = O_WRONLY | O_CREAT | O_TRUNC;

    const int log_fd = open(log_path.c_str(), open_flags, 0644);

    if (log_fd < 0)
    {
        return false;
    }

    if (dup2(log_fd, STDOUT_FILENO) < 0 || dup2(log_fd, STDERR_FILENO) < 0)
    {
        close(log_fd);
        return false;
    }

    if (log_fd != STDOUT_FILENO && log_fd != STDERR_FILENO)
    {
        close(log_fd);
    }

    std::setvbuf(stdout, nullptr, _IOLBF, 0);
    std::setvbuf(stderr, nullptr, _IOLBF, 0);

    return true;
}

bool Logger::open_log_file()
{
    std::lock_guard<std::mutex> lock(this->write_mutex);

    if (this->initialized)
    {
        return true;
    }

    this->log_file    = stdout;
    this->initialized = true;
    return true;
}

bool Logger::init()
{
    return Logger::instance().open_log_file();
}

void Logger::log_v(Level level, const char *fmt, va_list args)
{
    std::lock_guard<std::mutex> lock(this->write_mutex);

    if (!this->initialized || this->log_file == nullptr)
    {
        return;
    }

    const auto now = std::chrono::system_clock::now();
    const std::time_t t =
        std::chrono::system_clock::to_time_t(now);

    std::tm tm_buf {};
    localtime_r(&t, &tm_buf);

    char time_buf[32];
    std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", &tm_buf);

    std::fprintf(this->log_file,
                 "[%s] [%s] ",
                 time_buf,
                 this->level_label(level));

    std::vfprintf(this->log_file, fmt, args);
    std::fputc('\n', this->log_file);
    std::fflush(this->log_file);
}

void Logger::plain_v(const char *fmt, va_list args)
{
    std::lock_guard<std::mutex> lock(this->write_mutex);

    if (!this->initialized || this->log_file == nullptr)
    {
        return;
    }

    std::vfprintf(this->log_file, fmt, args);
    std::fputc('\n', this->log_file);
    std::fflush(this->log_file);
}

void Logger::log(Level level, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    Logger::instance().log_v(level, fmt, args);
    va_end(args);
}

void Logger::info(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    Logger::instance().log_v(Level::Info, fmt, args);
    va_end(args);
}

void Logger::warn(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    Logger::instance().log_v(Level::Warning, fmt, args);
    va_end(args);
}

void Logger::error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    Logger::instance().log_v(Level::Error, fmt, args);
    va_end(args);
}

void Logger::plain(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    Logger::instance().plain_v(fmt, args);
    va_end(args);
}
