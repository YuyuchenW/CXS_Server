#ifndef __CXS_LOG_H__
#define __CXS_LOG_H__
#include <string>
#include <memory>
#include <list>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <stdarg.h>
#include "util.h"
#include <unordered_map>
#include "singleton.h"
#include <set>
#include "thread.h"

#define CXS_LOG_LEVEL(logger, level)                                                                                        \
    if (logger->getLevel() <= level)                                                                                        \
    CXS::LogEventWrap(CXS::LogEvent::ptr(new CXS::LogEvent((char *)__FILE__, __LINE__, 0,                                   \
                                                           CXS::GetThreadId(), CXS::GetFiberId(), time(0), logger, level,CXS::Thread::GetName()))) \
        .getSS()
#define CXS_LOG_DEBUG(logger) CXS_LOG_LEVEL(logger, CXS::LogLevel::DEBUG)
#define CXS_LOG_INFO(logger) CXS_LOG_LEVEL(logger, CXS::LogLevel::INFO)
#define CXS_LOG_WARN(logger) CXS_LOG_LEVEL(logger, CXS::LogLevel::WARN)
#define CXS_LOG_ERROR(logger) CXS_LOG_LEVEL(logger, CXS::LogLevel::ERROR)
#define CXS_LOG_FATAL(logger) CXS_LOG_LEVEL(logger, CXS::LogLevel::FATAL)

#define CXS_LOG_FMT_LEVEL(logger, level, fmt, ...)                                                                          \
    if (logger->getLevel() <= level)                                                                                        \
    CXS::LogEventWrap(CXS::LogEvent::ptr(new CXS::LogEvent((char *)__FILE__, __LINE__, 0,                                   \
                                                           CXS::GetThreadId(), CXS::GetFiberId(), time(0), logger, level,CXS::Thread::GetName()))) \
        .getEvent()                                                                                                         \
        ->format(fmt, __VA_ARGS__)
#define CXS_LOG_FMT_DEBUG(logger, fmt, ...) CXS_LOG_FMT_LEVEL(logger, CXS::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define CXS_LOG_FMT_INFO(logger, fmt, ...) CXS_LOG_FMT_LEVEL(logger, CXS::LogLevel::INFO, fmt, __VA_ARGS__)
#define CXS_LOG_FMT_WARN(logger, fmt, ...) CXS_LOG_FMT_LEVEL(logger, CXS::LogLevel::WARN, fmt, __VA_ARGS__)
#define CXS_LOG_FMT_ERROR(logger, fmt, ...) CXS_LOG_FMT_LEVEL(logger, CXS::LogLevel::ERROR, fmt, __VA_ARGS__)
#define CXS_LOG_FMT_FATAL(logger, fmt, ...) CXS_LOG_FMT_LEVEL(logger, CXS::LogLevel::FATAL, fmt, __VA_ARGS__)
#define CXS_LOG_ROOT() CXS::LoggerMgr::GetInstance()->getRoot()
#define CXS_LOG_NAME(name) CXS::LoggerMgr::GetInstance()->getLogger(name);
namespace CXS
{
    class Logger;
    class LoggerManager;
    // 日志等级
    enum class LogLevel
    {
        UNKNOW = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };
    // 日志事件
    class LogEvent
    {

    public:
        LogEvent(char *fliename, uint32_t line, uint32_t elapse, uint32_t threadId,
                 uint32_t FiberId, uint32_t time, std::shared_ptr<Logger> logger, LogLevel level, const std::string& thread_name);
        const char *getFile() const { return m_file; }
        uint32_t getElapse() const { return m_elapse; }
        uint32_t getline() const { return m_line; }
        uint32_t getThread() const { return m_threadId; }
        uint32_t getFiber() const { return m_fiberId; }
        uint64_t getTime() const { return m_time; }
        const std::string& getThreadName() const {return m_threadName;}
        std::string getContent() const { return m_ss.str(); }
        std::stringstream &getStringStream() { return m_ss; }

        typedef std::shared_ptr<LogEvent> ptr;
        std::shared_ptr<Logger> getLogger() const { return m_logger; }
        LogLevel GetLevel() { return m_level; }
        void format(const char *fmt, ...);
        void format(const char *fmt, va_list al);

    private:
        const char *m_file = nullptr; // 文件名
        uint64_t m_line = 0;          // 行号
        uint32_t m_elapse = 0;        // 程序运行时间
        uint32_t m_threadId = 0;      // 线程id
        uint32_t m_fiberId = 0;       // 协程id
        uint64_t m_time = 0;          // 时间戳
        std::stringstream m_ss;       // 内容
        std::shared_ptr<Logger> m_logger;
        LogLevel m_level;
        std::string m_threadName;
    };

    class LogEventWrap
    {
    public:
        LogEventWrap(LogEvent::ptr e);
        ~LogEventWrap();
        std::stringstream &getSS();
        LogEvent::ptr getEvent() const { return m_event; }

    private:
        LogEvent::ptr m_event;
    };
    // 日志格式器
    class LogFormatter
    {
    public:
        typedef std::shared_ptr<LogFormatter> ptr;
        LogFormatter(const std::string &pattern);
        //%t    %thread_id%m%n
        std::string format(std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event);
        void init();
        bool isError() const { return m_error; }
        const std::string getPattern() const { return m_pattern; }

    public:
        class FormatItem
        {
        public:
            typedef std::shared_ptr<FormatItem> ptr;
            virtual ~FormatItem() {}
            virtual void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) = 0;
        };

    private:
        std::string m_pattern;
        std::vector<FormatItem::ptr> m_item;
        bool m_error = false;
    };
    // 日志输出地
    class LogAppender
    {
        friend class Logger;

    public:
        typedef CASLock MutexType;
        typedef std::shared_ptr<LogAppender> ptr;
        virtual ~LogAppender(){};
        virtual void log(std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) = 0;
        virtual std::string toYamlString() = 0;

        void setFormatter(LogFormatter::ptr formatter);
        LogFormatter::ptr getFormatter();
        LogLevel getLevel() { return m_level; }
        void setLevel(LogLevel val) { m_level = val; }

    protected:
        LogLevel m_level = LogLevel::DEBUG;
        LogFormatter::ptr m_formatter;
        bool m_hasFormatter = false;
        MutexType m_mutex;
    };

    // 日志器
    class Logger : public std::enable_shared_from_this<Logger>
    {
        friend class LoggerManager;

    public:
        typedef CASLock MutexType;
        typedef std::shared_ptr<Logger> ptr;
        Logger(const std::string &name = "root");
        void log(LogLevel level, LogEvent::ptr event);

        void debug(LogEvent::ptr event);
        void info(LogEvent::ptr event);
        void warn(LogEvent::ptr event);
        void error(LogEvent::ptr event);
        void fatal(LogEvent::ptr event);

        void addAppender(LogAppender::ptr appender);
        void delAppender(LogAppender::ptr appender);
        void clearAppenders();
        LogLevel getLevel() const { return m_level; }
        void setLevel(LogLevel level) { m_level = level; }
        std::string &getName() { return m_name; }

        void setFormatter(LogFormatter::ptr val);
        void setFormatter(const std::string &val);
        std::string toYamlString();
        LogFormatter::ptr getFormatter();

    private:
        std::string m_name;
        LogLevel m_level;
        std::list<LogAppender::ptr> m_appenders;
        LogFormatter::ptr m_formatter;
        Logger::ptr m_root;
        MutexType m_mutex;
    };
    // 输出到控制台 appender
    class StdoutAppender : public LogAppender
    {
    public:
        typedef std::shared_ptr<StdoutAppender> ptr;
        void log(std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) override;
        std::string toYamlString() override;
    };
    // 输出到文件
    class FileLogAppender : public LogAppender
    {
    public:
        typedef std::shared_ptr<FileLogAppender> ptr;
        FileLogAppender(const std::string &filename);
        void log(std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) override;
        std::string toYamlString() override;

        // 重新打开文件，成功则返回true
        bool reopen();

    private:
        std::string m_filename;
        std::ofstream m_filestream;
    };

    class LoggerManager
    {
    public:
        typedef CASLock MutexType;
        LoggerManager();
        Logger::ptr getLogger(const std::string &name);
        void init();
        Logger::ptr getRoot() { return m_root; };
        std::string toYamlString();

    private:
        std::unordered_map<std::string, Logger::ptr> m_loggers;
        Logger::ptr m_root;
        MutexType m_mutex;
    };

    typedef CXS::Singleton<LoggerManager> LoggerMgr;
} // namespace CX

#endif