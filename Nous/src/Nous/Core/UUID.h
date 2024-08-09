#pragma once

#include <xhash>

namespace Nous {

    class UUID
    {
    public:
        UUID(); // 创建对象即生成UUID
        UUID(uint64_t uuid);
        UUID(const UUID&) = default;

        operator uint64_t() const { return m_UUID; }
    private:
        uint64_t m_UUID;
    };

}

namespace std {

    // 提供哈希函数，可以给unordered_map等依赖哈希算法的容器使用
    template<>
    struct hash<Nous::UUID>
    {
        std::size_t operator()(const Nous::UUID& uuid) const
        {
            return hash<uint64_t>()((uint64_t)uuid);
        }
    };
}
