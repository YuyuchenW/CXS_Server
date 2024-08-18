#include "../http/http_parser.h"
#include "../code/log.h"
#include <cstddef>

static CXS::Logger::ptr g_logger = CXS_LOG_ROOT();

const char test_request_data[] = "GET / HTTP/1.0\r\n"
                                 "Host: www.sylar.top\r\n"
                                 "Content-Length: 10\r\n\r\n"
                                 "1234567890";

void test_request() {
    CXS::http::HttpRequestParser parser;
    std::string tmp = test_request_data;
    size_t s = parser.execute(&tmp[0], tmp.size());
    CXS_LOG_DEBUG(g_logger) << "execute rt = " << s << " has error = " << parser.hasError()
                            << " is_finished = " << parser.isFinished()
                            << " total = " << tmp.size()
                            << " content length = " << parser.getContentLength();
    tmp.resize(tmp.size() - s);

    CXS_LOG_INFO(g_logger) << parser.getData()->toString();
}

const char test_response_data[] = "HTTP/1.1 200 OK\r\n"
                                  "Date: Tue, 04 Jun 2019 15:43:56 GMT\r\n"
                                  "Server: Apache\r\n"
                                  "Last-Modified: Tue, 12 Jan 2010 13:48:00 GMT\r\n"
                                  "ETag: \"51-47cf7e6ee8400\"\r\n"
                                  "Accept-Ranges: bytes\r\n"
                                  "Content-Length: 81\r\n"
                                  "Cache-Control: max-age=86400\r\n"
                                  "Expires: Wed, 05 Jun 2019 15:43:56 GMT\r\n"
                                  "Connection: Close\r\n"
                                  "Content-Type: text/html\r\n\r\n"
                                  "<html>\r\n"
                                  "<meta http-equiv=\"refresh\" content=\"0;url=http://www.baidu.com/\">\r\n"
                                  "</html>\r\n";

void test_response() {
    CXS::http::HttpResponseParser parser;
    std::string tmp = test_response_data;
    size_t s = parser.execute(&tmp[0], tmp.size());
    CXS_LOG_INFO(g_logger) << "execute rt = " << s << " has error = " << parser.hasError()
                           << " is_finished = " << parser.isFinished()
                           << " total = " << tmp.size()
                           << " content length = " << parser.getContentLength();

    tmp.resize(tmp.size() - s);
    CXS_LOG_INFO(g_logger) << parser.getData()->toString();
    CXS_LOG_INFO(g_logger) << tmp;
}

int main(int argc, char **argv) {
    test_response();
    return 0;
}