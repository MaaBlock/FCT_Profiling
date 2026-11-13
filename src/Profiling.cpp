#include "./Profiling.h"

namespace FCT {

    namespace {
#if defined(FCT_PROFILING_ENABLED) && FCT_PROFILING_ENABLED && defined(TRACY_ENABLE)
        void tracyMemoryAllocHook(const void* ptr, size_t size) noexcept {
            if (ptr && size > 0) {
                TracyAlloc(ptr, size);
            }
        }

        void tracyMemoryFreeHook(const void* ptr) noexcept {
            if (ptr) {
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
            setMemoryProfileHooks(&tracyMemoryAllocHook, &tracyMemoryFreeHook);
            setThreadNameProfileHook(&tracySetThreadNameHook);
#else
            setMemoryProfileHooks(nullptr, nullptr);
            setThreadNameProfileHook(nullptr);
#endif
        }

        void shutdown() noexcept {
            setMemoryProfileHooks(nullptr, nullptr);
            setThreadNameProfileHook(nullptr);
        }

    } // namespace Profiling

} // namespace FCT

// Note: 静态初始化阶段即完成钩子注入，并通过该导出符号防止连接器裁剪 Profiling.obj
extern "C" volatile int fct_profiling_auto_init_anchor = (FCT::Profiling::init(), 1);
