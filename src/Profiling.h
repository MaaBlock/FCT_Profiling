#ifndef FCT_PROFILING_PROFILING_H
#define FCT_PROFILING_PROFILING_H

#include <FCT_Base.h>

#if !defined(FCT_PROFILING_FORCE_DISABLE) && ((defined(FCT_PROFILING_ENABLED) && FCT_PROFILING_ENABLED) || (!defined(FCT_PROFILING_ENABLED) && defined(TRACY_ENABLE)))
#define FCT_PROFILING_ACTIVE 1
#else
#define FCT_PROFILING_ACTIVE 0
#endif

#if FCT_PROFILING_ACTIVE && defined(TRACY_ENABLE)

#include <tracy/Tracy.hpp>
#include <tracy/TracyC.h>

#undef ZoneScoped
// Note: 沿用无变量阴影包装宏，避免嵌套 ZoneScoped() 在高告警级别下触发变量名遮蔽告警
#define ZoneScoped() SuppressVarShadowWarning( ZoneNamed( ___tracy_scoped_zone, true ) )

namespace FCT {
    using tracy::SetThreadName;
} // namespace FCT

#else

#define ZoneNamed(x, y) ((void)0)
#define ZoneNamedN(x, y, z) ((void)0)
#define ZoneNamedC(x, y, z) ((void)0)
#define ZoneNamedNC(x, y, z, w) ((void)0)

#define ZoneTransient(x, y) ((void)0)
#define ZoneTransientN(x, y, z) ((void)0)

#define ZoneScoped() ((void)0)
#define ZoneScopedN(x) ((void)0)
#define ZoneScopedC(x) ((void)0)
#define ZoneScopedNC(x, y) ((void)0)

#define ZoneText(x, y) ((void)0)
#define ZoneTextV(x, y, z) ((void)0)
#define ZoneTextF(x, ...) ((void)0)
#define ZoneTextVF(x, y, ...) ((void)0)
#define ZoneName(x, y) ((void)0)
#define ZoneNameV(x, y, z) ((void)0)
#define ZoneNameF(x, ...) ((void)0)
#define ZoneNameVF(x, y, ...) ((void)0)
#define ZoneColor(x) ((void)0)
#define ZoneColorV(x, y) ((void)0)
#define ZoneValue(x) ((void)0)
#define ZoneValueV(x, y) ((void)0)
#define ZoneIsActive false
#define ZoneIsActiveV(x) false

#define FrameMark ((void)0)
#define FrameMarkNamed(x) ((void)0)
#define FrameMarkStart(x) ((void)0)
#define FrameMarkEnd(x) ((void)0)
#define FrameImage(x, y, z, w, a) ((void)0)

#define TracyPlot(x, y) ((void)0)
#define TracyPlotConfig(x, y, z, w, a) ((void)0)
#define TracyMessage(x, y) ((void)0)
#define TracyMessageL(x) ((void)0)
#define TracyMessageC(x, y, z) ((void)0)
#define TracyMessageLC(x, y) ((void)0)
#define TracyAppInfo(x, y) ((void)0)

#define TracyAlloc(x, y) ((void)0)
#define TracyFree(x) ((void)0)
#define TracyAllocN(x, y, z) ((void)0)
#define TracyFreeN(x, y) ((void)0)
#define TracyIsConnected false
#define TracyIsStarted false
#define TracySetProgramName(x) ((void)0)

namespace FCT {
    inline void SetThreadName(const char*) noexcept {}
} // namespace FCT

#endif

namespace FCT::Profiling {

    /**
     * @brief 初始化 FCT_Profiling 并向 FCT_Base 绑定性能剖析观测回调
     */
    void init() noexcept;

    /**
     * @brief 解绑注册到 FCT_Base 的性能剖析观测回调
     */
    void shutdown() noexcept;

} // namespace FCT::Profiling

#if FCT_PROFILING_ACTIVE && defined(_MSC_VER)
#  if defined(_M_X64) || defined(_M_ARM64)
#    pragma comment(linker, "/INCLUDE:fct_profiling_auto_init_anchor")
#  else
#    pragma comment(linker, "/INCLUDE:_fct_profiling_auto_init_anchor")
#  endif
#endif

#endif // FCT_PROFILING_PROFILING_H
