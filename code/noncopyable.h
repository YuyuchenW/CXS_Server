#ifndef __CXS_NONCOPYABLE_H__
#define __CXS_NONCOPYABLE_H__

namespace CXS {
class Noncopyable {
public:
    Noncopyable(const Noncopyable &) = delete;
    Noncopyable &operator=(const Noncopyable &) = delete;
    Noncopyable() = default;
    ~Noncopyable() = default;
};
} // namespace CXS
#endif // __CXS_NONCOPYABLE_H__