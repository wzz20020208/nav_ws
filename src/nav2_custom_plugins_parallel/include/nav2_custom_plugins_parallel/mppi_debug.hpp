#ifndef NAV2_CUSTOM_PLUGINS__MPPI_DEBUG_HPP_
#define NAV2_CUSTOM_PLUGINS__MPPI_DEBUG_HPP_

// 调试开关：设为1启用MPPI调试输出，设为0禁用
#define MPPI_DEBUG 1

#if MPPI_DEBUG
#include <cstdio>
#include <cstdarg>

static inline void mppi_debug(const char* fmt, ...) {
    FILE* f = fopen("/tmp/mppi_debug.log", "a");
    if (f) {
        va_list args;
        va_start(args, fmt);
        vfprintf(f, fmt, args);
        va_end(args);
        fprintf(f, "\n");
        fclose(f);
    }
}
#define MPPI_DBG mppi_debug
#else
#define MPPI_DBG(...) ((void)0)
#endif

#endif  // NAV2_CUSTOM_PLUGINS__MPPI_DEBUG_HPP_