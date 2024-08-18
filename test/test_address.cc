#include "../code/hook.h"
#include "../code/iomanager.h"
#include "../code/log.h"
#include "../code/address.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>
CXS::Logger::ptr g_logger = CXS_LOG_ROOT();
void test() {
    CXS_LOG_INFO(g_logger) << "begin";
    std::vector<CXS::Address::ptr> addrs;
    bool v = CXS::Address::Lookup(addrs, "www.sylar.top:http");
    if (!v) {
        CXS_LOG_ERROR(g_logger) << "lookup failed";
        return;
    }

    for (size_t i = 0; i < addrs.size(); ++i) {
        CXS_LOG_INFO(g_logger) << "addr: " << addrs[i]->toString();
    }
}

void test_iface() {
    std::multimap<std::string, std::pair<CXS::Address::ptr, uint32_t>> results;
    bool v = CXS::Address::GetInterfaceAddress(results);
    if (!v) {
        CXS_LOG_ERROR(g_logger) << "get interface address failed";
        return;
    }
    for (auto it = results.begin(); it != results.end(); ++it) {
        CXS_LOG_INFO(g_logger) << it->first << " " << it->second.first->toString()
                               << " " << it->second.second;
    }
}

void test_ipv4() {
    auto addr = CXS::IPAddress::Create("www.sylar.top");
    if (addr) {
        CXS_LOG_INFO(g_logger) << addr->toString();
    }
}
int main(int argc, char *argv[]) {
    test_ipv4();
    return 0;
}