#include "../http/http.h"
#include "../code/log.h"
#include <iostream>
#include <new>

void test_request() {
    CXS::http::HttpRequest::ptr req(new CXS::http::HttpRequest);
    req->setHeader("host", "www.sylar.top");
    req->setBody("hello world");
    req->setVersion(0x10);
    req->dump(std::cout) << std::endl;
}

void test_response() {
    CXS::http::HttpResponse::ptr req(new CXS::http::HttpResponse);
    req->setHeader("X-X", "www.sylar.top");
    req->setBody("hello world");
    req->setClose(false);
    req->setStatus((CXS::http::HttpStatus)400);
    req->dump(std::cout) << std::endl;
}

int main(int arg, char *argv[]) {
    test_request();
    test_response();
    return 0;
}