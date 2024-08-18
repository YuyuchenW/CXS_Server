#include "bytearray.h"
#include <bits/types/struct_iovec.h>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include "endian.h"
#include "log.h"
#include <cstring>
#include <fstream>
#include <ios>
#include <memory.h>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <iomanip>
namespace CXS {
static CXS::Logger::ptr g_logger = CXS_LOG_NAME("system");
ByteArray::Node::Node(size_t s) :
    data(new char[s]),
    size(s),
    next(nullptr) {
}

ByteArray::Node::Node() :
    data(nullptr),
    size(0),
    next(nullptr) {
}

ByteArray::Node::~Node() {
    if (data) {
        delete[] data;
    }
}
ByteArray::ByteArray(size_t base_size) :
    m_position(0),
    m_capacity(base_size),
    m_baseSize(base_size),
    m_size(0),
    m_endian(CXS_BIG_ENDIAN),
    m_root(new Node(base_size)),
    m_current(m_root) {
}
ByteArray::~ByteArray() {
    Node *tmp = m_root;
    while (tmp) {
        m_current = tmp;
        tmp = tmp->next;
        delete m_current;
    }
}

bool ByteArray::isLittleEndian() const {
    return m_endian == CXS_LITTLE_ENDIAN;
};

void ByteArray::setLittleEndian(bool val) {
    m_endian = val ? CXS_LITTLE_ENDIAN : CXS_BIG_ENDIAN;
}
// write
void ByteArray::writeFint8(int8_t value) {
    write(&value, sizeof(value));
}
void ByteArray::writeFuint8(uint8_t value) {
    write(&value, sizeof(value));
}
void ByteArray::writeFint16(int16_t value) {
    if (m_endian != CXS_BYTE_ORDER) {
        value = CXS::byteswap(value);
    }
    write(&value, sizeof(value));
}
void ByteArray::writeFuint16(uint16_t value) {
    if (m_endian != CXS_BYTE_ORDER) {
        value = CXS::byteswap(value);
    }
    write(&value, sizeof(value));
}
void ByteArray::writeFint32(int32_t value) {
    if (m_endian != CXS_BYTE_ORDER) {
        value = CXS::byteswap(value);
    }
    write(&value, sizeof(value));
}
void ByteArray::writeFuint32(uint32_t value) {
    if (m_endian != CXS_BYTE_ORDER) {
        value = CXS::byteswap(value);
    }
    write(&value, sizeof(value));
}
void ByteArray::writeFint64(int64_t value) {
    if (m_endian != CXS_BYTE_ORDER) {
        value = CXS::byteswap(value);
    }
    write(&value, sizeof(value));
}
void ByteArray::writeFuint64(uint64_t value) {
    if (m_endian != CXS_BYTE_ORDER) {
        value = CXS::byteswap(value);
    }
    write(&value, sizeof(value));
}
static uint32_t EncodeZigzag32(const int32_t &v) {
    if (v < 0) {
        return ((uint32_t)(-v)) * 2 - 1;
    } else {
        return v * 2;
    }
}

static uint32_t EncodeZigzag64(const int64_t &v) {
    if (v < 0) {
        return ((uint64_t)(-v)) * 2 - 1;
    } else {
        return v * 2;
    }
}

static int32_t DecodeZigzag32(const uint32_t &v) {
    return (v >> 1) ^ -((v & 1));
}

static int64_t DecodeZigzag64(const uint64_t &v) {
    return (v >> 1) ^ -((v & 1));
}
void ByteArray::writeint32(int32_t value) {
    writeuint32(EncodeZigzag32(value));
}
void ByteArray::writeuint32(uint32_t value) {
    uint8_t tmp[5];
    uint8_t i = 0;
    while (value >= 0x80) {
        tmp[i++] = (value & 0x7f) | 0x80;
        value >>= 7;
    }
    tmp[i++] = value;
    write(&tmp, i);
}
void ByteArray::writeint64(int64_t value) {
    writeuint64(EncodeZigzag64(value));
}
void ByteArray::writeuint64(uint64_t value) {
    uint8_t tmp[10];
    uint8_t i = 0;
    while (value >= 0x80) {
        tmp[i++] = (value & 0x7f) | 0x80;
        value >>= 7;
    }
    tmp[i++] = value;
    write(&tmp, i);
}

void ByteArray::writeFloat(float value) {
    uint32_t v;
    memcpy(&v, &value, sizeof(value));
    writeFuint32(v);
}
void ByteArray::writeDouble(double value) {
    uint64_t v;
    memcpy(&v, &value, sizeof(value));
    writeFuint64(v);
}
// length: int16, data
void ByteArray::writeStringF16(std::string value) {
    writeFuint16(value.size());
    write(value.c_str(), value.size());
}
// length: int32, data
void ByteArray::writeStringF32(std::string value) {
    writeFuint32(value.size());
    write(value.c_str(), value.size());
}
// length: int64, data
void ByteArray::writeStringF64(std::string value) {
    writeFuint64(value.size());
    write(value.c_str(), value.size());
}
// length: varint, data
void ByteArray::writeStringVint(std::string value) {
    writeFuint64(value.size());
    write(value.c_str(), value.size());
}
// data
void ByteArray::writeStringWithoutLength(std::string value) {
    write(value.c_str(), value.size());
}

// read
int8_t ByteArray::readFint8() {
    int8_t v;
    read(&v, sizeof(v));
    return v;
}
uint8_t ByteArray::readFuint8() {
    int8_t v;
    read(&v, sizeof(v));
    return v;
}
#define XX(type)                      \
    type v;                           \
    read(&v, sizeof(v));              \
    if (m_endian == CXS_BYTE_ORDER) { \
        return v;                     \
    } else {                          \
        return CXS::byteswap(v);      \
    }

int16_t ByteArray::readFint16() {
    XX(int16_t);
}
uint16_t ByteArray::readFuint16() {
    XX(uint16_t);
}
int32_t ByteArray::readFint32() {
    XX(int32_t);
}
uint32_t ByteArray::readFuint32() {
    XX(uint32_t);
}
int64_t ByteArray::readFint64() {
    XX(int64_t);
}
uint64_t ByteArray::readFuint64() {
    XX(uint64_t);
}

int32_t ByteArray::readint32() {
    return DecodeZigzag32(readuint32());
}
uint32_t ByteArray::readuint32() {
    uint32_t result = 0;
    for (int i = 0; i < 32; i += 7) {
        uint8_t b = readFuint8();
        if (b < 0x80) {
            result |= ((uint32_t)b) << i;
            break;
        } else {
            result |= ((uint32_t)(b & 0x7f)) << i;
        }
    }
    return result;
}
int64_t ByteArray::readint64() {
    return DecodeZigzag64(readuint64());
}
uint64_t ByteArray::readuint64() {
    uint64_t result = 0;
    for (int i = 0; i < 64; i += 7) {
        uint8_t b = readFuint8();
        if (b < 0x80) {
            result |= ((uint64_t)b) << i;
            break;
        } else {
            result |= ((uint64_t)(b & 0x7f)) << i;
        }
    }
    return result;
}

float ByteArray::readFloat() {
    uint32_t v = readFuint32();
    float value;
    memcpy(&value, &v, sizeof(v));
    return v;
}
double ByteArray::readDouble() {
    uint64_t v = readFuint32();
    double value;
    memcpy(&value, &v, sizeof(v));
    return v;
}

// length: int16, data
std::string ByteArray::readStringF16() {
    uint16_t len = readFuint16();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
}
// length: int32, data
std::string ByteArray::readStringF32() {
    uint32_t len = readFuint32();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
}
// length: int64, data
std::string ByteArray::readStringF64() {
    uint64_t len = readFuint64();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
} // length: varint, data
std::string ByteArray::readStringVint() {
    uint64_t len = readFuint64();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
}

// 内部操作
void ByteArray::clear() {
    m_position = m_size = 0;
    m_capacity = m_baseSize;
    Node *tmp = m_root->next;
    while (tmp) {
        m_current = tmp;
        tmp = tmp->next;
        delete m_current;
    }
    m_current = m_root;
    m_current->next = nullptr;
}

void ByteArray::write(const void *buf, size_t size) {
    if (size == 0) {
        return;
    }
    addCapacity(size);

    // 当前位置对于当前块的偏移量
    size_t npos = m_position % m_baseSize;

    // 当前块剩余容量
    size_t ncap = m_current->size - npos;
    // 要写入的数据块偏移量
    size_t bpos = 0;
    while (size > 0) {
        if (ncap >= size) {
            memcpy(m_current->data + npos, static_cast<const char *>(buf) + bpos, size);
            m_position += size;
            if (m_current->size == (npos + size)) {
                m_current = m_current->next;
            }
            bpos += size;
            size = 0;
        } else {
            memcpy(m_current->data + npos, static_cast<const char *>(buf) + bpos, ncap);
            m_position += ncap;
            bpos += ncap;
            size -= ncap;
            m_current = m_current->next;
            ncap = m_current->size;
            npos = 0;
        }
    }

    if (m_position > m_size) {
        m_size = m_position;
    }
}
void ByteArray::read(void *buf, size_t size) {
    if (size > getReadSize()) {
        throw std::out_of_range("read out of range");
    }
    // 当前位置对于当前块的偏移量
    size_t npos = m_position % m_baseSize;
    // 当前块剩余容量
    size_t ncap = m_current->size - npos;
    // 要读取的数据块偏移量
    size_t bpos = 0;

    while (size > 0) {
        //该节点剩余的数据都读到buf中
        if (ncap >= size) {
            memcpy(static_cast<char *>(buf) + bpos, m_current->data + npos, size);
            if (m_current->size == (npos + size)) {
                m_current = m_current->next;
            }
            m_position += size;
            bpos += size;
            size = 0;
        } else {
            memcpy(static_cast<char *>(buf) + bpos, m_current->data + npos, size);
            m_position += ncap;
            bpos += ncap;
            size -= ncap;
            m_current = m_current->next;
            ncap = m_current->size;
            npos = 0;
        }
    }
}

void ByteArray::read(void *buf, size_t size, size_t position) const {
    if (size > getReadSize()) {
        throw std::out_of_range("read out of range");
    }

    // 当前位置对于当前块的偏移量
    size_t npos = position % m_baseSize;
    // 当前块剩余容量
    size_t ncap = m_current->size - npos;
    // 要读取的数据块偏移量
    size_t bpos = 0;
    Node *cur = m_current;
    while (size > 0) {
        //该节点剩余的数据都读到buf中
        if (ncap >= size) {
            memcpy(static_cast<char *>(buf) + bpos, cur->data + npos, size);
            if (cur->size == (npos + size)) {
                cur = cur->next;
            }
            position += size;
            bpos += size;
            size = 0;
        } else {
            memcpy(static_cast<char *>(buf) + bpos, cur->data + npos, size);
            position += ncap;
            bpos += ncap;
            size -= ncap;
            cur = cur->next;
            ncap = cur->size;
            npos = 0;
        }
    }
}

void ByteArray::setPosition(size_t val) {
    if (val > m_capacity) {
        throw std::out_of_range("set position out of range");
    }

    m_position = val;
    if (m_position > m_size) {
        m_size = m_position;
    }
    m_current = m_root;
    while (val >= m_current->size) {
        val -= m_current->size;
        m_current = m_current->next;
    }
    if (val == m_current->size) {
        m_current = m_current->next;
    }
}

bool ByteArray::writeToFile(const std::string &name) const {
    std::ofstream ofs;
    ofs.open(name, std::ios::trunc | std::ios::binary);
    if (!ofs) {
        CXS_LOG_ERROR(g_logger) << "writeToFile name = " << name
                                << "error, error no = " << errno << "error str = " << strerror(errno);
        return false;
    }

    int64_t read_size = getReadSize();
    int64_t pos = m_position;
    Node *cur = m_current;
    while (read_size > 0) {
        int diff = pos % m_baseSize;
        int64_t len = (read_size > (int64_t)m_baseSize ? m_baseSize : read_size) - diff;
        ofs.write(cur->data + diff, len);
        cur = cur->next;
        pos += len;
        read_size -= len;
    }
    return true;
}
bool ByteArray::readFromFile(const std::string &name) {
    std::ifstream ifs;
    ifs.open(name, std::ios::binary);
    if (!ifs) {
        CXS_LOG_ERROR(g_logger) << "readFromFile name = " << name
                                << "error, error no = " << errno << "error str = " << strerror(errno);
        return false;
    }
    std::shared_ptr<char> buff(new char[m_baseSize], [](char *ptr) { delete[] ptr; });
    while (!ifs.eof()) {
        ifs.read(buff.get(), m_baseSize);
        write(buff.get(), ifs.gcount());
    }
    return true;
}

void ByteArray::addCapacity(size_t size) {
    if (size == 0) {
        return;
    }
    size_t old_cap = getCapacity();
    if (old_cap >= size) {
        return;
    }
    size = size - old_cap;
    size_t count = (size / m_baseSize) + ((size % m_baseSize) > old_cap ? 1 : 0);
    Node *tmp = m_root;
    while (tmp->next) {
        tmp = tmp->next;
    }
    Node *first = nullptr;
    for (size_t i = 0; i < count; ++i) {
        tmp->next = new Node(m_baseSize);
        if (first == nullptr) {
            first = tmp->next;
        }
        tmp = tmp->next;
        m_capacity += m_baseSize;
    }
    if (old_cap == 0) {
        m_current = first;
    }
}

std::string ByteArray::toString() const {
    std::string str;
    str.resize(getReadSize());
    if (str.empty()) {
        return str;
    }
    read(&str[0], str.size(), m_position);
    return str;
}

std::string ByteArray::toHexString() const {
    std::string str = toString();
    std::stringstream ss;
    for (size_t i = 0; i < str.size(); ++i) {
        if (i > 0 && i % 32 == 0) {
            ss << std::endl;
        }
        ss << std::setw(2) << std::setfill('0') << std::hex << (int)(uint8_t)str[i] << " ";
    }
    return ss.str();
}

uint64_t ByteArray::getReadBuffers(std::vector<iovec> &buffers, uint64_t len) const {
    len = len > getReadSize() ? getReadSize() : len;
    if (len == 0) {
        return 0;
    }
    uint64_t size = len;
    size_t npos = m_position % m_baseSize;
    size_t ncap = m_current->size - npos;
    struct iovec iov;
    Node *cur = m_current;
    while (len > 0) {
        if (ncap >= len) {
            iov.iov_base = cur->data + npos;
            iov.iov_len = len;
            len = 0;
        } else {
            iov.iov_base = cur->data + npos;
            iov.iov_len += ncap;
            len -= ncap;
            cur = cur->next;
            ncap = cur->size;
            npos = 0;
        }
    }
    buffers.push_back(iov);
    return size;
}
uint64_t ByteArray::getReadBuffers(std::vector<iovec> &buffers, uint64_t len, uint64_t position) const {
    if (position > m_size) {
        throw std::out_of_range("set position out of range");
    }
    len = len > getReadSize() ? getReadSize() : len;
    if (len == 0) {
        return 0;
    }
    uint64_t size = len;
    size_t npos = position % m_baseSize;
    size_t count = position / m_baseSize;
    Node *cur = m_root;

    while (count > 0) {
        cur = cur->next;
        --count;
    }
    size_t ncap = m_current->size - npos;
    struct iovec iov;
    while (len > 0) {
        if (ncap >= len) {
            iov.iov_base = cur->data + npos;
            iov.iov_len = len;
            len = 0;
        } else {
            iov.iov_base = cur->data + npos;
            iov.iov_len = ncap;
            len -= ncap;
            cur = cur->next;
            ncap = cur->size;
            npos = 0;
        }
        buffers.push_back(iov);
    }
    return size;
}
uint64_t ByteArray::getWriteBuffers(std::vector<iovec> &buffers, uint64_t len) {
    if (len == 0) {
        return 0;
    }
    addCapacity(len);
    uint64_t size = len;
    size_t npos = m_position % m_baseSize;
    size_t ncap = m_current->size - npos;
    struct iovec iov;
    Node *cur = m_current;
    while (len > 0) {
        if (ncap >= len) {
            iov.iov_base = cur->data + npos;
            iov.iov_len = len;
            len = 0;
        } else {
            iov.iov_base = cur->data + npos;
            iov.iov_len = ncap;
            len -= ncap;
            cur = cur->next;
            ncap = cur->size;
            npos = 0;
        }
        buffers.push_back(iov);
    }
    return size;
};
} // namespace CXS