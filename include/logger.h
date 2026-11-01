#pragma once

#include <cstdarg>
#include <cstdio>
#include <filesystem>
#include <mutex>

class Logger
{
public:
    enum class Level
    {
        Info,
        Warning,
        Error,
    };

    static bool init();

    static void log(Level level, const char *fmt, ...)
        __attribute__((format(printf, 2, 3)));

    static void info(const char *fmt, ...)
        __attribute__((format(printf, 1, 2)));

    static void warn(const char *fmt, ...)
        __attribute__((format(printf, 1, 2)));

    static void error(const char *fmt, ...)
        __attribute__((format(printf, 1, 2)));

    static void plain(const char *fmt, ...)
        __attribute__((format(printf, 1, 2)));

private:
    static Logger &instance();

    bool open_log_file();

    bool redirect_stdio(const std::filesystem::path &path);

    std::filesystem::path default_log_path() const;

    const char *level_label(Level level) const;

    void log_v(Level level, const char *fmt, va_list args);

    void plain_v(const char *fmt, va_list args);

    std::mutex write_mutex;
    FILE      *log_file    = nullptr;
    bool       initialized = false;

    static constexpr const char log_file_name[] = ".host.log";
};
