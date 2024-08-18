#include "../http/http_server.h"
#include "../code/log.h"
#include <unistd.h>
static CXS::Logger::ptr g_logger = CXS_LOG_ROOT();

void run() {
    CXS::http::HttpServer::ptr server(new CXS::http::HttpServer);
    CXS::Address::ptr addr = CXS::Address::LookupAny("0.0.0.0:8020");
    while (!server->bind(addr)) {
        sleep(2);
    }
    server->start();
}
int main(int argc, char *argv[]) {
    CXS::IOManager iom(2);
    iom.schedule(run);
    return 0;
}