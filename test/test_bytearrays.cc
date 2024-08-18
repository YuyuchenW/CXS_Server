#include "../code/bytearray.h"
#include "../code/log.h"
#include "../code/macro.h"
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>
static CXS::Logger::ptr g_logger = CXS_LOG_ROOT();

void test() {
#define XX(type, len, write_fun, read_fun, base_len)                                           \
    {                                                                                          \
        std::vector<type> vec;                                                                 \
        for (int i = 0; i < len; ++i) {                                                        \
            vec.push_back(rand());                                                             \
        }                                                                                      \
        CXS::ByteArray::ptr ba(new CXS::ByteArray(base_len));                                  \
        for (auto &i : vec) {                                                                  \
            ba->write_fun(i);                                                                  \
        }                                                                                      \
        ba->setPosition(0);                                                                    \
        for (size_t i = 0; i < vec.size(); ++i) {                                              \
            type v = ba->read_fun();                                                           \
            CXS_ASSERT(v == vec[i]);                                                           \
        }                                                                                      \
        CXS_ASSERT(ba->getReadSize() == 0);                                                    \
        CXS_LOG_INFO(g_logger) << #write_fun "/" #read_fun "(" #type ") len  = " << len        \
                               << ", base_len = " << base_len << ", size = " << ba->getSize(); \
    }
    XX(int8_t, 100, writeFint8, readFint8, 1);
    XX(int8_t, 100, writeFuint8, readFuint8, 1);
    XX(int16_t, 100, writeFint16, readFint16, 1);
    XX(uint16_t, 100, writeFuint16, readFuint16, 1);
    XX(int32_t, 100, writeFint32, readFint32, 1);
    XX(uint32_t, 100, writeFuint32, readFuint32, 1);
    XX(int64_t, 100, writeFint64, readFint64, 1);
    XX(uint64_t, 100, writeFuint64, readFuint64, 1);

    XX(int32_t, 100, writeint32, readint32, 1);
    XX(uint32_t, 100, writeuint32, readuint32, 1);
    XX(int64_t, 100, writeint64, readint64, 1);
    XX(uint64_t, 100, writeuint64, readuint64, 1);
#undef XX

#define XX(type, len, write_fun, read_fun, base_len)                                                        \
    {                                                                                                       \
        std::vector<type> vec;                                                                              \
        for (int i = 0; i < len; ++i) {                                                                     \
            vec.push_back(rand());                                                                          \
        }                                                                                                   \
        CXS::ByteArray::ptr ba(new CXS::ByteArray(base_len));                                               \
        for (auto &i : vec) {                                                                               \
            ba->write_fun(i);                                                                               \
        }                                                                                                   \
        ba->setPosition(0);                                                                                 \
        for (size_t i = 0; i < vec.size(); ++i) {                                                           \
            type v = ba->read_fun();                                                                        \
            CXS_ASSERT(v == vec[i]);                                                                        \
        }                                                                                                   \
        CXS_ASSERT(ba->getReadSize() == 0);                                                                 \
        CXS_LOG_INFO(g_logger) << #write_fun "/" #read_fun "(" #type ") len  = " << len                     \
                               << ", base_len = " << base_len << ", size = " << ba->getSize();              \
        ba->setPosition(0);                                                                                 \
        ba->writeToFile("/test_bytearray" #type "_" #len "_" #write_fun "_" #read_fun ".dat");              \
        CXS::ByteArray::ptr ba2(new CXS::ByteArray(len * 2));                                               \
        CXS_ASSERT(ba2->readFromFile("/test_bytearray" #type "_" #len "_" #write_fun "_" #read_fun ".dat")) \
        ba2->setPosition(0);                                                                                \
        CXS_ASSERT(ba->toString() == ba2->toString());                                                      \
        CXS_ASSERT(ba->getPosition() == 0);                                                                 \
        CXS_ASSERT(ba2->getPosition() == 0);                                                                \
    }

    XX(int8_t, 100, writeFint8, readFint8, 1);
    XX(int8_t, 100, writeFuint8, readFuint8, 1);
    XX(int16_t, 100, writeFint16, readFint16, 1);
    XX(uint16_t, 100, writeFuint16, readFuint16, 1);
    XX(int32_t, 100, writeFint32, readFint32, 1);
    XX(uint32_t, 100, writeFuint32, readFuint32, 1);
    XX(int64_t, 100, writeFint64, readFint64, 1);
    XX(uint64_t, 100, writeFuint64, readFuint64, 1);

    XX(int32_t, 100, writeint32, readint32, 1);
    XX(uint32_t, 100, writeuint32, readuint32, 1);
    XX(int64_t, 100, writeint64, readint64, 1);
    XX(uint64_t, 100, writeuint64, readuint64, 1);
#undef XX
}

int main(int argc, char **argv) {
    test();
    return 0;
}