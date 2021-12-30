#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

#include "SafetyHook.hpp"
#include "ThreadFreezer.hpp"

class SafetyHookFactory : public std::enable_shared_from_this<SafetyHookFactory> {
public:
    struct ActiveFactory {
        std::shared_ptr<SafetyHookFactory> factory{};
        std::scoped_lock<std::mutex> mux_lock;
        ThreadFreezer threads{};

        ActiveFactory(std::shared_ptr<SafetyHookFactory> f) : factory{f}, mux_lock{factory->m_mux} {
            factory->m_active_factory = this;
        }
        ~ActiveFactory() { factory->m_active_factory = nullptr; }

        std::unique_ptr<SafetyHook> create(void* target, void* destination) {
            return factory->create(target, destination);
        }

        std::shared_ptr<SafetyHook> create_shared(void* target, void* destination) {
            return factory->create_shared(target, destination);
        }
    };

    static auto init() { return std::shared_ptr<SafetyHookFactory>{new SafetyHookFactory}; }

    ActiveFactory acquire();

private:
    friend SafetyHook;

    struct FreeNode {
        std::unique_ptr<FreeNode> next{};
        uintptr_t start{};
        uintptr_t end{};
    };

    struct MemoryAllocation {
        uintptr_t address{};
        size_t size{};
        std::unique_ptr<FreeNode> freelist{};

        ~MemoryAllocation();
    };

    std::vector<std::unique_ptr<MemoryAllocation>> m_allocations{};
    std::mutex m_mux{};
    ActiveFactory* m_active_factory{};

    SafetyHookFactory() = default;

    std::unique_ptr<SafetyHook> create(void* target, void* destination);
    std::shared_ptr<SafetyHook> create_shared(void* target, void* destination);

    uintptr_t allocate(size_t size);
    uintptr_t allocate_near(
        const std::vector<uintptr_t>& desired_addresses, size_t size, size_t max_distance = 0x7FFF'FFFF);
    void free(uintptr_t address, size_t size);

    void combine_adjacent_freenodes(MemoryAllocation& allocation);
    uintptr_t allocate_nearby_memory(const std::vector<uintptr_t>& desired_addresses, size_t size, size_t max_distance);
    bool in_range(uintptr_t address, const std::vector<uintptr_t>& desired_addresses, size_t max_distance);
};