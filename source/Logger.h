/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2026 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef NATIVEPLAYER_LOGGER_H
#define NATIVEPLAYER_LOGGER_H
#include <cstdlib>
#include <iostream>
#include <string>

namespace nativeplayer
{
    enum class LogLevel
    {
        TRACE = 0,
        DEBUG = 1,
        INFO  = 2,
        ERROR = 3
    };

    inline bool equalsIgnoreCase(const std::string &lhs, const std::string &rhs)
    {
        if (lhs.size() != rhs.size())
        {
            return false;
        }

        for (std::size_t i = 0; i < lhs.size(); ++i)
        {
            const char a = lhs[i];
            const char b = rhs[i];
            if (a == b)
            {
                continue;
            }

            const char aLower = (a >= 'A' && a <= 'Z') ? static_cast<char>(a + ('a' - 'A')) : a;
            const char bLower = (b >= 'A' && b <= 'Z') ? static_cast<char>(b + ('a' - 'A')) : b;
            if (aLower != bLower)
            {
                return false;
            }
        }

        return true;
    }

    // Default logging is intentionally quiet; only errors are emitted unless overridden.
    inline LogLevel CURRENT_LOG_LEVEL = LogLevel::ERROR;

    inline LogLevel parseLogLevel(const std::string &value)
    {
        if (equalsIgnoreCase(value, "trace")) return LogLevel::TRACE;
        if (equalsIgnoreCase(value, "debug")) return LogLevel::DEBUG;
        if (equalsIgnoreCase(value, "info")) return LogLevel::INFO;
        if (equalsIgnoreCase(value, "error")) return LogLevel::ERROR;

        return LogLevel::ERROR;
    }

    inline LogLevel getConfiguredLogLevel()
    {
        const char *envValue = std::getenv("NATIVEPLAYER_LOG_LEVEL");
        if (!envValue)
        {
            envValue = std::getenv("LOG_LEVEL");
        }

        if (!envValue)
        {
            return LogLevel::ERROR;
        }

        return parseLogLevel(envValue);
    }

    //Provide a method to set the log level at runtime if needed. This can be useful for changing the verbosity without recompiling.
    inline void setLogLevel(LogLevel level)
    {
        CURRENT_LOG_LEVEL = level;
    }

    //Method to convert LogLevel enum to string for better readability in logs.
    inline std::string logLevelToString(LogLevel level)
    {
        switch (level)
        {
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::ERROR:
            return "ERROR";
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::TRACE:
            return "TRACE";
        default:
            return "UNKNOWN";
        }
    }

    template <typename T, typename... Args>
    inline void LOG(LogLevel level, const T &firstArg, const Args &...args)
    {
        if(level >= CURRENT_LOG_LEVEL)
        {
            std::cout << "[NativePlayer][" << logLevelToString(level) << "] " << firstArg;
            using expander = int[];
            (void)expander{0, (void(std::cout << args), 0)...};
            std::cout << std::endl;
        }
    }

} // namespace nativeplayer
#endif // NATIVEPLAYER_LOGGER_H
