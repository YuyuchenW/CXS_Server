#ifndef __CXS__ENDIAN_H__
#define __CXS__ENDIAN_H__

#include <cstdint>
#include <type_traits>
#define CXS_LITTLE_ENDIAN 1
#define CXS_BIG_ENDIAN 2

#include <byteswap.h>
#include <stdint.h>

namespace CXS {
template <class T>
typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type
byteswap(T value) {
    return (T)bswap_64((uint64_t)value);
}

template <class T>
typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type
byteswap(T value) {
    return (T)bswap_32((uint32_t)value);
}

template <class T>
typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type
byteswap(T value) {
    return (T)bswap_16((uint16_t)value);
}

#if BYTE_ORDER == BIG_ENDIAN
#define CXS_BYTE_ORDER CXS_BIG_ENDIAN
#else
#define CXS_BYTE_ORDER CXS_LITTLE_ENDIAN
#endif

#if CXS_BYTE_ORDER == CXS_BIG_ENDIAN

/**
 * @brief 只在小端机器上执行byteswap, 在大端机器上什么都不做
 */
template <class T>
T byteswapOnLittleEndian(T t) {
    return t;
}

/**
 * @brief 只在大端机器上执行byteswap, 在小端机器上什么都不做
 */
template <class T>
T byteswapOnBigEndian(T t) {
    return byteswap(t);
}
#else

/**
 * @brief 只在小端机器上执行byteswap, 在大端机器上什么都不做
 */
template <class T>
T byteswapOnLittleEndian(T t) {
    return byteswap(t);
}

/**
 * @brief 只在大端机器上执行byteswap, 在小端机器上什么都不做
 */
template <class T>
T byteswapOnBigEndian(T t) {
    return t;
}
#endif
} // namespace CXS

#endif