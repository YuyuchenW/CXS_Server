#include "http.h"
#include <cstdint>
#include <ostream>
#include <sstream>
#include <string.h>
#include <string>
#include <strings.h>

namespace CXS {

namespace http {
HttpMethod StringToHttpMethod(const std::string &method) {
#define XX(num, name, string)                   \
    if (strcmp(#string, method.c_str()) == 0) { \
        return HttpMethod::name;                \
    }
    HTTP_METHOD_MAP(XX)
#undef XX
    return HttpMethod::HTTP_INVALID_METHOD;
}
HttpMethod CharsToHttpMethod(const char *method) {
#define XX(num, name, string)                             \
    if (strncmp(#string, method, strlen(#string)) == 0) { \
        return HttpMethod::name;                          \
    }
    HTTP_METHOD_MAP(XX)
#undef XX
    return HttpMethod::HTTP_INVALID_METHOD;
}

static const char *s_method_strings[] = {
#define XX(num, name, string) #string,
    HTTP_METHOD_MAP(XX)
#undef XX
};

const char *HttpMethodToString(const HttpMethod &method) {
    uint32_t idx = (uint32_t)method;
    if (idx >= sizeof(s_method_strings) / sizeof(char *)) {
        return "<unknown>";
    }
    return s_method_strings[idx];
}

const char *HttpStatusToString(const HttpStatus &status) {
    switch (status) {
#define XX(num, name, string) \
    case HttpStatus::name:    \
        return #string;
        HTTP_STATUS_MAP(XX);
#undef XX
    default:
        return "<unknown>";
    }
}
bool CaseInsensitiveLess::operator()(const std::string &lhs,
                                     const std::string &rhs) const {
    return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
}

HttpRequest::HttpRequest(uint8_t version, bool close) :
    m_method(HttpMethod::GET), m_version(version),
    m_close(close), m_path("/") {
}

std::string HttpRequest::getHeaders(const std::string &key, const std::string &def) const {
    auto it = m_headers.find(key);
    if (it == m_headers.end()) {
        return def;
    }
    return it->second;
}
std::string HttpRequest::getParams(const std::string &key, const std::string &def) const {
    auto it = m_params.find(key);
    if (it == m_params.end()) {
        return def;
    }
    return it->second;
}
std::string HttpRequest::getCookies(const std::string &key, const std::string &def) const {
    auto it = m_cookies.find(key);
    if (it == m_cookies.end()) {
        return def;
    }
    return it->second;
}

void HttpRequest::setHeader(const std::string &key, const std::string &value) {
    m_headers[key] = value;
}
void HttpRequest::setParam(const std::string &key, const std::string &value) {
    m_params[key] = value;
}
void HttpRequest::setCookie(const std::string &key, const std::string &value) {
    m_cookies[key] = value;
}

void HttpRequest::delHeader(const std::string &key) {
    m_headers.erase(key);
}
void HttpRequest::delParam(const std::string &key) {
    m_params.erase(key);
}
void HttpRequest::delCookies(const std::string &key) {
    m_cookies.erase(key);
}

bool HttpRequest::hasHeader(const std::string &key, std::string *val) {
    auto it = m_headers.find(key);
    if (it == m_headers.end()) {
        return false;
    }
    if (val) {
        *val = it->second;
    }
    return true;
}
bool HttpRequest::hasParam(const std::string &key, std::string *val) {
    auto it = m_params.find(key);
    if (it == m_params.end()) {
        return false;
    }
    if (val) {
        *val = it->second;
    }
    return true;
}
bool HttpRequest::hasCookie(const std::string &key, std::string *val) {
    auto it = m_cookies.find(key);
    if (it == m_cookies.end()) {
        return false;
    }
    if (val) {
        *val = it->second;
    }
    return true;
}

std::ostream &HttpRequest::dump(std::ostream &os) const {
    // Get /uri  HTTP/1.1
    //  Host : www.example.com

    os << HttpMethodToString(m_method) << " " << m_path << (m_query.empty() ? "" : "?") << m_query
       << (m_fragment.empty() ? "" : "#") << m_fragment << " HTTP/" << ((uint32_t)m_version >> 4) << "."
       << ((uint32_t)m_version & 0x0f) << "\r\n";
    os << "connection: " << (m_close ? "close" : "keep-alive") << "\r\n";
    for (auto &header : m_headers) {
        if (strcasecmp(header.first.c_str(), "connection") == 0) {
            continue;
        }
        os << header.first << ": " << header.second << "\r\n";
    }
    if (!m_body.empty()) {
        os << "content-length: " << m_body.size() << "\r\n\r\n"
           << m_body;
    } else {
        os << "\r\n"
           << m_body;
    }
    return os;
}

std::string HttpRequest::toString() const {
    std::stringstream ss;
    dump(ss);
    return ss.str();
}

HttpResponse::HttpResponse(uint8_t version, bool close) :
    m_status(HttpStatus::OK), m_version(version), m_close(close) {
}

std::ostream &HttpResponse::dump(std::ostream &os) const {
    os << "HTTP/"
       << ((uint32_t)m_version >> 4) << "."
       << ((uint32_t)m_version & 0x0F) << " "
       << (uint32_t)m_status << " "
       << (m_reason.empty() ? HttpStatusToString(m_status) : m_reason) << "\r\n";

    for (auto &it : m_headers) {
        if (strcasecmp(it.first.c_str(), "connection") == 0) {
            continue;
        }
        os << it.first << ": " << it.second << "\r\n";
    }
    os << "connection: " << (m_close ? "close" : "keep-alive") << "\r\n";

    if (!m_body.empty()) {
        os << "content-length: " << m_body.size() << "\r\n\r\n"
           << m_body;
    } else {
        os << "\r\n";
    }
    return os;
}

std::string HttpResponse::toString() const {
    std::stringstream ss;
    dump(ss);
    return ss.str();
}
std::string HttpResponse::getHeader(const std::string &key, const std::string &def) const {
    auto it = m_headers.find(key);
    return it == m_headers.end() ? def : it->second;
}
void HttpResponse::setHeader(const std::string &key, const std::string &val) {
    m_headers[key] = val;
}
void HttpResponse::delHeader(const std::string &key) {
    m_headers.erase(key);
}
std::ostream &operator<<(std::ostream &os, const HttpRequest &req) {
    return req.dump(os);
}
std::ostream &operator<<(std::ostream &os, const HttpResponse &req) {
    return req.dump(os);
}
}
} // namespace CXS::http