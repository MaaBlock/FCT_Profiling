#include "./Profiling.h"

namespace FCT {

    namespace {
#if defined(FCT_PROFILING_ENABLED) && FCT_PROFILING_ENABLED && defined(TRACY_ENABLE)
        struct TracyMemoryTrackerState {
            std::mutex mutex;
            std::unordered_set<const void*> trackedPtrs;
        };

        // Note: 采用静态原始存储配合定位构造驻留进程全生命周期，既避免触发 FCT_NEW 递归，也防止退出期静态析构顺序导致悬垂访问
        TracyMemoryTrackerState& getTracyMemoryTrackerState() noexcept {
            alignas(TracyMemoryTrackerState) static std::byte s_storage[sizeof(TracyMemoryTrackerState)];
            static TracyMemoryTrackerState* s_state = ::new (static_cast<void*>(s_storage)) TracyMemoryTrackerState();
            return *s_state;
        }

        void tracyMemoryAllocHook(const void* ptr, size_t size) noexcept {
            if (!ptr || size == 0) {
                return;
            }
            TracyMemoryTrackerState& state = getTracyMemoryTrackerState();
            std::lock_guard<std::mutex> lock(state.mutex);
            const auto [_, inserted] = state.trackedPtrs.insert(ptr);
            if (!inserted) {
                TracyFree(ptr);
            }
            TracyAlloc(ptr, size);
        }

        void tracyMemoryFreeHook(const void* ptr) noexcept {
            if (!ptr) {
                return;
            }
            TracyMemoryTrackerState& state = getTracyMemoryTrackerState();
            std::lock_guard<std::mutex> lock(state.mutex);
            if (state.trackedPtrs.erase(ptr) > 0) {
                TracyFree(ptr);
            }
        }

        void tracySetThreadNameHook(const char* name) noexcept {
            if (name) {
                tracy::SetThreadName(name);
            }
        }
#endif
    } // namespace

    namespace Profiling {

        void init() noexcept {
#if defined(FCT_PROFILING_ENABLED) && FCT_PROFILING_ENABLED && defined(TRACY_ENABLE)
            MemoryTracker::setCallbacks(&tracyMemoryAllocHook, &tracyMemoryFreeHook);
            ThreadPool::setThreadNameCallback(&tracySetThreadNameHook);
#else
            MemoryTracker::setCallbacks(nullptr, nullptr);
            ThreadPool::setThreadNameCallback(nullptr);
#endif
        }

        void shutdown() noexcept {
            MemoryTracker::setCallbacks(nullptr, nullptr);
            ThreadPool::setThreadNameCallback(nullptr);
        }

    } // namespace Profiling

} // namespace FCT

// Note: 静态初始化阶段即完成回调绑定，并通过该导出符号防止链接器裁剪 Profiling.obj
extern "C" volatile int fct_profiling_auto_init_anchor = (FCT::Profiling::init(), 1);
