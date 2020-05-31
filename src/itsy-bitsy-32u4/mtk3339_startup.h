#ifndef _MTK3339_STARTUP_INCLUDE_GUARD
#define _MTK3339_STARTUP_INCLUDE_GUARD

namespace mtk3339
{
    using size_type = unsigned;
    using time_type = unsigned long;

    enum class ReturnCode
    {
        Ok = 0,
        StartupError = 1,
        ConfigError = 2
    };

    ReturnCode init();
}

#endif // _MTK3339_STARTUP_INCLUDE_GUARD
