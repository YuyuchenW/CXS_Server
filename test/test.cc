#include<iostream>
#include "../code/log.h"
#include"../code/util.h"
#include<thread>
int main(int argc, char const *argv[])
{
    CXS::Logger::ptr logger(new CXS::Logger);
    logger->addAppender(CXS::LogAppender::ptr(new CXS::StdoutAppender));
    // CXS::LogEvent::ptr event(new CXS::LogEvent(__FILE__,__LINE__,1,CXS::GetThreadId(),CXS::GetFiberId(),time(0)));
    // event->getStringStream()<<"hello cxs log";
    // logger->log(CXS::LogLevel::DEBUG,event);
    CXS::FileLogAppender::ptr file_appender(new CXS::FileLogAppender("./log.txt"));
    CXS::LogFormatter::ptr fmt(new CXS::LogFormatter("%d%T%m%n"));
    file_appender ->setFormatter(fmt);
    file_appender ->setLevel(CXS::LogLevel::ERROR);
    logger->addAppender(file_appender);

    CXS_LOG_INFO(logger) <<" cxs : hello log";
    CXS_LOG_ERROR(logger) <<"there will erro!";
    CXS_LOG_FATAL(logger) <<"sys down";
    CXS_LOG_WARN(logger) <<"warn";
    CXS_LOG_DEBUG(logger) <<"debug";

    CXS_LOG_FMT_DEBUG(logger,"test fmt debug %s","some erro");

    auto l = CXS::LoggerMgr::GetInstance()->getLogger("cxs");
    CXS_LOG_INFO(l)<<"XXXXX";
    return 0;
}
