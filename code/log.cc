#include "log.h"
#include <tuple>
#include <map>
#include <functional>
#include <time.h>
#include "config.hpp"
namespace CXS
{
    const char *LogLevelToString(LogLevel level)
    {
        switch (level)
        {
#define XX(name)         \
    case LogLevel::name: \
        return #name;    \
        break;

            XX(DEBUG);
            XX(INFO);
            XX(WARN);
            XX(ERROR);
            XX(FATAL);
#undef XX
        default:
            return "UNKNOW";
        }
        return "UNKONW";
    }

    LogLevel fromsStringToLevel(const std::string &str)
    {
#define XX(level, strlevel)     \
    if (str == #strlevel)       \
    {                           \
        return LogLevel::level; \
    }
        XX(DEBUG, debug);
        XX(INFO, info);
        XX(WARN, warn);
        XX(ERROR, error);
        XX(FATAL, fatal);

        XX(DEBUG, DEBUG);
        XX(INFO, INFO);
        XX(WARN, WARN);
        XX(ERROR, ERROR);
        XX(FATAL, FATAL);

        return LogLevel::UNKNOW;
#undef XX
    }
    LogEvent::LogEvent(char *fliename, uint32_t line, uint32_t elapse, uint32_t threadId,
                       uint32_t FiberId, uint32_t time, std::shared_ptr<Logger> logger, LogLevel level, const std::string& threadName)
        : m_file(fliename), m_line(line), m_elapse(elapse), m_threadId(threadId),
          m_fiberId(FiberId), m_time(time), m_logger(logger), m_level(level),m_threadName(threadName)
    {
    }

    void LogEvent::format(const char *fmt, ...)
    {
        va_list al;
        va_start(al, fmt);
        format(fmt, al);
        va_end(al);
    }

    void LogEvent::format(const char *fmt, va_list al)
    {
        char *buf = nullptr;
        int len = vasprintf(&buf, fmt, al);
        if (len != -1)
        {
            m_ss << std::string(buf, len);
            free(buf);
        }
    }

    LogEventWrap::LogEventWrap(LogEvent::ptr e)
        : m_event(e) {}
    LogEventWrap::~LogEventWrap()
    {
        m_event->getLogger()->log(m_event->GetLevel(), m_event);
    }
    std::stringstream &LogEventWrap::getSS()
    {
        return m_event->getStringStream();
    }

    Logger::Logger(const std::string &name)
    {
        m_name = name;
        m_level = LogLevel::DEBUG;
        m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
    }

    void Logger::log(LogLevel level, LogEvent::ptr event)
    {
        if (level >= m_level)
        {

            auto self = shared_from_this();
            MutexType::Lock lock(m_mutex);

            if (!m_appenders.empty())
            {
                for (auto &i : m_appenders)
                {
                    i->log(self, level, event);
                }
            }
            else if (m_root)
            {
                m_root->log(level, event);
            }
        }
    }

    class MessageFormatItem : public LogFormatter::FormatItem
    {
    public:
        MessageFormatItem(const std::string &fmt = ""){};
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) override
        {
            os << event->getContent();
        }
    };

    class LevelFormatItem : public LogFormatter::FormatItem
    {
    public:
        LevelFormatItem(const std::string &fmt = ""){};
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) override
        {
            os << LogLevelToString(level);
        }
    };

    class ElapseFormatItem : public LogFormatter::FormatItem
    {
    public:
        ElapseFormatItem(const std::string &fmt = ""){};
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) override
        {
            os << event->getElapse();
        }
    };

    class NameFormatItem : public LogFormatter::FormatItem
    {
    public:
        NameFormatItem(const std::string &fmt = ""){};
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) override
        {
            os << event->getLogger()->getName();
        }
    };

    class ThreadIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        ThreadIdFormatItem(const std::string &fmt = ""){};
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) override
        {
            os << event->getThread();
        }
    };
    class FiberIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        FiberIdFormatItem(const std::string &fmt = ""){};
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) override
        {
            os << event->getFiber();
        }
    };

    class ThreadNameFormatItem : public LogFormatter::FormatItem
    {
    public:
        ThreadNameFormatItem(const std::string &str = ""){};
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) override
        {
            os << event->getThreadName();
        }
    };


    class DateTimeFormatItem : public LogFormatter::FormatItem
    {
    public:
        DateTimeFormatItem(const std::string &format = "%Y-%m-%d %H:%M:%S") : m_format(format)
        {
            if (m_format.empty())
            {
                m_format = "%Y-%m-%d %H:%M:%S";
            }
        };
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) override
        {
            struct tm tm;
            time_t time = event->getTime();
            localtime_r(&time, &tm);
            char buf[64];
            strftime(buf, sizeof(buf), m_format.c_str(), &tm);
            os << buf;
        }

    private:
        std::string m_format;
    };

    class FilenameFormatItem : public LogFormatter::FormatItem
    {
    public:
        FilenameFormatItem(const std::string &fmt = ""){};
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) override
        {
            os << event->getFile();
        }
    };

    class LineFormatItem : public LogFormatter::FormatItem
    {
    public:
        LineFormatItem(const std::string &fmt = ""){};
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) override
        {
            os << event->getline();
        }
    };

    class NewLineFormatItem : public LogFormatter::FormatItem
    {
    public:
        NewLineFormatItem(const std::string &str){};
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) override
        {
            os << std::endl;
        }
    };

    class StringFormatItem : public LogFormatter::FormatItem
    {
    public:
        StringFormatItem(const std::string &str) : m_string(str){};
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) override
        {
            os << m_string;
        }

    private:
        std::string m_string;
    };

    class TabFormatItem : public LogFormatter::FormatItem
    {
    public:
        TabFormatItem(const std::string &str){};
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event) override
        {
            os << '\t';
        }
    };

    void Logger::debug(LogEvent::ptr event)
    {
        log(LogLevel::DEBUG, event);
    }
    void Logger::info(LogEvent::ptr event)
    {
        log(LogLevel::INFO, event);
    }
    void Logger::error(LogEvent::ptr event)
    {
        log(LogLevel::ERROR, event);
    }
    void Logger::fatal(LogEvent::ptr event)
    {
        log(LogLevel::FATAL, event);
    }
    void Logger::warn(LogEvent::ptr event)
    {
        log(LogLevel::WARN, event);
    }

    void Logger::delAppender(LogAppender::ptr appender)
    {
        MutexType::Lock lock(m_mutex);
        for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it)
        {
            if (*it == appender)
            {
                m_appenders.erase(it);
                break;
            }
        }
    }

    void Logger::clearAppenders()
    {
        MutexType::Lock lock(m_mutex);
        m_appenders.clear();
    }

    void Logger::setFormatter(LogFormatter::ptr val)
    {
        MutexType::Lock lock(m_mutex);
        m_formatter = val;
        for (auto &i : m_appenders)
        {
            MutexType::Lock lc(i->m_mutex);
            if (!i->m_hasFormatter)
            {
                i->m_formatter = m_formatter;
            }
        }
    }

    void Logger::setFormatter(const std::string &val)
    {
        CXS::LogFormatter::ptr new_val(new CXS::LogFormatter(val));
        if (new_val->isError())
        {
            std::cout << "[error] Logger setFormatter name = " << m_name
                      << "value = " << val << " invalid formatter";
            return;
        }
        // m_formatter = new_val;
        setFormatter(new_val);
    }

    LogFormatter::ptr Logger::getFormatter()
    {
        MutexType::Lock lock(m_mutex);
        return m_formatter;
    }

    void LogAppender::setFormatter(LogFormatter::ptr formatter)
    {
        MutexType::Lock lock(m_mutex);
        m_formatter = formatter;
        if (m_formatter)
        {
            m_hasFormatter = true;
        }
        else
        {
            m_hasFormatter = false;
        }
    }

    LogFormatter::ptr LogAppender::getFormatter()
    {
        MutexType::Lock lock(m_mutex);
        return m_formatter;
    }

    void Logger::addAppender(LogAppender::ptr appender)
    {
        MutexType::Lock lock(m_mutex);

        if (!appender->getFormatter())
        {
            MutexType::Lock lk(appender->m_mutex);
            appender->m_formatter = m_formatter;
        }
        m_appenders.push_back(appender);
    }

    std::string Logger::toYamlString()
    {
        MutexType::Lock lock(m_mutex);
        YAML::Node node;
        node["name"] = m_name;
        node["level"] = LogLevelToString(m_level);
        if (m_formatter)
        {
            node["formatter"] = m_formatter->getPattern();
        }

        for (auto &i : m_appenders)
        {
            node["appenders"].push_back(YAML::Load(i->toYamlString()));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    void StdoutAppender::log(std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event)
    {
        if (level >= m_level)
        {
            MutexType::Lock lock(m_mutex);
            std::cout << m_formatter->format(logger, level, event);
        }
    }

    std::string StdoutAppender::toYamlString()
    {
        MutexType::Lock lock(m_mutex);
        YAML::Node node;
        node["type"] = "StdoutLogAppender";
        node["level"] = LogLevelToString(m_level);
        if (m_formatter && m_hasFormatter)
        {
            node["formatter"] = m_formatter->getPattern();
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
    FileLogAppender::FileLogAppender(const std::string &filename)
    {
        m_filename = filename;
        reopen();
    }

    bool FileLogAppender::reopen()
    {
        MutexType::Lock lock(m_mutex);

        if (m_filestream)
        {
            m_filestream.close();
        }
        m_filestream.open(m_filename, std::ios_base::app | std::ios_base::out);
        return !!m_filestream;
    }

    void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event)
    {
        if (level >= m_level)
        {
            MutexType::Lock lock(m_mutex);
            m_filestream << m_formatter->format(logger, level, event);
        }
    }
    std::string FileLogAppender::toYamlString()
    {
        MutexType::Lock lock(m_mutex);
        YAML::Node node;
        node["type"] = "FileLogAppender";
        node["file"] = m_filename;
        node["level"] = LogLevelToString(m_level);
        if (m_formatter && m_hasFormatter)
        {
            node["formatter"] = m_formatter->getPattern();
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    LogFormatter::LogFormatter(const std::string &pattern)
    {
        m_pattern = pattern;
        init();
    }

    std::string LogFormatter::format(std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event)
    {
        std::stringstream ss;
        for (auto &i : m_item)
        {
            i->format(ss, logger, level, event);
        }
        return ss.str();
    }
    // 格式化  %xxx %xxx{xxx} %%
    void LogFormatter::init()
    {
        // str,format,type
        std::vector<std::tuple<std::string, std::string, int>> fmtVec;
        std::string nstr;
        for (size_t i = 0; i < m_pattern.size(); ++i)
        {
            // 获取%前的字符
            if (m_pattern[i] != '%')
            {
                nstr.append(1, m_pattern[i]);
                continue;
            }
            // 找到'%%'
            if (i + 1 < m_pattern.size())
            {
                if (m_pattern[i + 1] == '%')
                {
                    nstr.append(1, '%');
                    continue;
                }
            }
            //%后字符的位置
            size_t n = i + 1;
            int fmt_status = 0;
            size_t fmt_begin = 0;

            std::string str;
            std::string fmt;

            while (n < m_pattern.size())
            {
                if (!fmt_status && (!isalpha(m_pattern[n]) && m_pattern[n] != '{' && m_pattern[n] != '}'))
                {
                    str = m_pattern.substr(i + 1, n - i - 1);
                    break;
                }
                if (fmt_status == 0)
                {
                    // 截取'{'前的字符串，将fmt状态置为1 str
                    if (m_pattern[n] == '{')
                    {
                        str = m_pattern.substr(i + 1, n - i - 1);
                        // std::cout << "*" << str << std::endl;
                        fmt_status = 1; // 解析格式
                        fmt_begin = n;
                        ++n;
                        continue;
                    }
                }
                // 截取'{'与'}'之间的字符串 fmt_str
                else if (fmt_status == 1)
                {
                    if (m_pattern[n] == '}')
                    {
                        fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                        fmt_status = 0;
                        ++n;
                        break;
                    }
                }
                ++n;
                if (n == m_pattern.size())
                {
                    if (str.empty())
                    {
                        str = m_pattern.substr(i + 1);
                    }
                }
            }

            if (fmt_status == 0)
            {
                if (!nstr.empty())
                {
                    fmtVec.push_back(std::make_tuple(nstr, "", 0));
                    nstr.clear();
                }

                fmtVec.push_back(std::make_tuple(str, fmt, 1));
                i = n - 1;
            }
            else if (fmt_status == 1)
            {
                std::cout << "pattern parse error: " << m_pattern << "-" << m_pattern.substr(i) << std::endl;
                fmtVec.push_back(std::make_tuple("<<pattern error>>", fmt, 0));
                m_error = true;
            }
        }
        if (!nstr.empty())
        {
            fmtVec.push_back(std::make_tuple(nstr, "", 0));
        }

        static std::map<std::string, std::function<FormatItem::ptr(const std::string &str)>> s_format_items =
            {
#define XX(str, C)                                                               \
    {                                                                            \
        #str, [](const std::string &fmt) { return FormatItem::ptr(new C(fmt)); } \
    }

                XX(m, MessageFormatItem),  //%m --消息体
                XX(c, NameFormatItem),     //%c --日志名称
                XX(p, LevelFormatItem),    //%p --level
                XX(r, ElapseFormatItem),   //%r --启动后的时间
                XX(t, ThreadIdFormatItem), //%t --线程id
                XX(n, NewLineFormatItem),  //%n --回车换行
                XX(d, DateTimeFormatItem), //%d --时间
                XX(f, FilenameFormatItem), //%f --文件名
                XX(l, LineFormatItem),     //%l --行号
                XX(T, TabFormatItem),      //%T --制表符
                XX(F, FiberIdFormatItem),  //%F --协程id
                XX(N, ThreadNameFormatItem),//%N --线程name
                                           // XX(N, ThreadIdFormatItem)  //%N --线程id
#undef XX
            };

        for (auto i : fmtVec)
        {
            if (std::get<2>(i) == 0)
            {
                m_item.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
            }
            else
            {
                auto it = s_format_items.find(std::get<0>(i));
                if (it == s_format_items.end())
                {
                    m_item.push_back(FormatItem::ptr(new StringFormatItem("<<ERROR FORMAT %" + std::get<0>(i) + ">>")));
                    m_error = true;
                }
                else
                {
                    m_item.push_back(it->second(std::get<1>(i)));
                }
            }
            // std::cout << "(" << std::get<0>(i) << ") - (" << std::get<1>(i) << ") - (" << std::get<2>(i) << ")" << std::endl;
        }
    }

    LoggerManager::LoggerManager()
    {
        m_root.reset(new Logger);

        m_root->addAppender(LogAppender::ptr(new StdoutAppender));

        m_loggers[m_root->m_name] = m_root;
        init();
    }

    Logger::ptr LoggerManager::getLogger(const std::string &name)
    {
        MutexType::Lock lock(m_mutex);
        auto item = m_loggers.find(name);
        if (item != m_loggers.end())
        {
            return item->second;
        }
        Logger::ptr logger(new Logger(name));
        logger->m_root = m_root;
        m_loggers[name] = logger;
        return logger;
    }

    struct LogAppenderDefine
    {
        // 1 File, 2 stdcout
        int type = 0;
        LogLevel level = LogLevel::UNKNOW;
        std::string file;
        std::string formatter;

        bool operator==(const LogAppenderDefine &oth) const
        {
            return type == oth.type &&
                   level == oth.level &&
                   file == oth.file;
        }
    };

    struct LogDefine
    {
        std::string name;
        LogLevel level = LogLevel::UNKNOW;
        std::string formatter;
        std::vector<LogAppenderDefine> appenders;

        bool operator==(const LogDefine &oth) const
        {
            return name == oth.name &&
                   level == oth.level &&
                   formatter == oth.formatter &&
                   appenders == oth.appenders;
        };

        bool operator<(const LogDefine &oth) const
        {
            return name < oth.name;
        }
        bool isValid() const
        {
            return !name.empty();
        }
    };
    // 对LogDefine进行偏特化
    template <>
    class LexicalCast<std::string, LogDefine>
    {
    public:
        LogDefine operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            LogDefine ld;

            if (!node["name"].IsDefined())
            {
                std::cout << "log config error: name is null, " << node << std::endl;
                throw std::logic_error("log config name is null");
            }
            ld.name = node["name"].as<std::string>();
            ld.level = fromsStringToLevel(node["level"].IsDefined() ? node["level"].as<std::string>() : "");
            if (node["formatter"].IsDefined())
            {
                ld.formatter = node["formatter"].as<std::string>();
            }

            if (node["appenders"].IsDefined())
            {
                for (size_t x = 0; x < node["appenders"].size(); ++x)
                {
                    auto ap = node["appenders"][x];
                    if (!ap["type"].IsDefined())
                    {
                        std::cout << " log config error : appender type is null, " << ap << std::endl;
                        continue;
                    }

                    std::string type = ap["type"].as<std::string>();
                    LogAppenderDefine lad;
                    if (type == "FileLogAppender")
                    {
                        lad.type = 1;
                        if (!ap["file"].IsDefined())
                        {
                            std::cout << "log config error : fileappender file is null," << ap << std::endl;
                            continue;
                        }
                        lad.file = ap["file"].as<std::string>();
                        if (ap["formatter"].IsDefined())
                        {
                            lad.formatter = ap["formatter"].as<std::string>();
                        }
                    }
                    else if (type == "StdoutLogAppender")
                    {
                        lad.type = 2;
                        if (ap["formatter"].IsDefined())
                        {
                            lad.formatter = ap["formatter"].as<std::string>();
                        }
                    }
                    else
                    {
                        std::cout << "log config error: appender type is invalid, " << ap
                                  << std::endl;
                        continue;
                    }
                    ld.appenders.push_back(lad);
                }
            }
            return ld;
        }
    };

    template <>
    class LexicalCast<LogDefine, std::string>
    {
    public:
        std::string operator()(const LogDefine &i)
        {
            YAML::Node node;
            node["name"] = i.name;
            if (i.level != LogLevel::UNKNOW)
            {
                node["level"] = LogLevelToString(i.level);
            }

            if (!i.formatter.empty())
            {
                node["formatter"] = i.formatter;
            }
            for (auto &ap : i.appenders)
            {
                YAML::Node apNode;
                if (ap.type == 1)
                {
                    apNode["type"] = "FileLogAppender";
                    apNode["file"] = ap.file;
                }
                else if (ap.type == 2)
                {
                    apNode["type"] = "StdoutLogAppender";
                }
                if (ap.level != LogLevel::UNKNOW)
                {
                    apNode["level"] = LogLevelToString(ap.level);
                }

                if (!ap.formatter.empty())
                {
                    apNode["formatter"] = ap.formatter;
                }

                node["appenders"].push_back(apNode);
            }

            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    CXS::ConfigVar<std::set<LogDefine>>::ptr g_log_defines = CXS::Config::Lookup("logs", std::set<LogDefine>(), "logs config");

    struct LogIniter
    {
        LogIniter()
        {
            g_log_defines->addListener( [](const std::set<LogDefine> &old_value,
                                                const std::set<LogDefine> &new_value)
                                       {
                CXS_LOG_INFO(CXS_LOG_ROOT()) << "on logger_conf_changed";
                //add
                for(auto& i : new_value)
                {
                    auto it = old_value.find(i);
                    CXS::Logger::ptr logger;
                    if(it == old_value.end())
                    {
                        //add logger
                        //logger.reset(new CXS::Logger());
                        logger = CXS_LOG_NAME(i.name);
                    }
                    else{
                        if(!(i ==  *it)){
                            //modify
                            logger = CXS_LOG_NAME(i.name);
                        }
                    }
                    logger->setLevel(i.level);
                        if(!i.formatter.empty())
                        {
                            logger->setFormatter(i.formatter);
                        }
                        logger->clearAppenders();
                        for(auto & a : i.appenders)
                        {
                            CXS::LogAppender::ptr appender;
                            if(a.type ==1)
                            {
                                appender.reset(new FileLogAppender(a.file));
                            }
                            else if(a.type ==2)
                            {
                                appender.reset(new StdoutAppender);
                            }
                            appender->setLevel(a.level);
                            if(!a.formatter.empty())
                            {
                                LogFormatter::ptr fmt(new LogFormatter(a.formatter));
                                if(!fmt->isError())
                                {
                                    appender->setFormatter(fmt);
                                }
                                else
                                {
                                    std::cout<< "logger name = " << logger->getName()
                                    << "appender type : " << a.type << "formatter = " << a.formatter << "is invalid"<<std::endl;
                                }
                            }
                            logger->addAppender(appender);
                        }
                }

                for(auto& i : old_value)
                {
                    auto it = new_value.find(i);
                    if(it == new_value.end())
                    {
                        //delete
                        auto logger = CXS_LOG_NAME(i.name);
                        logger->setLevel((LogLevel)100);
                        logger->clearAppenders();
                    }
                } });
        }
    };

    static LogIniter __log_init;
    std::string LoggerManager::toYamlString()
    {
        MutexType::Lock lock(m_mutex);
        YAML::Node node;
        for (auto &i : m_loggers)
        {
            node.push_back(YAML::Load(i.second->toYamlString()));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
    void LoggerManager::init(){

    };
} // namespace CXS
