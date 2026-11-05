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
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>

namespace nativeplayer
{
    enum class LogLevel
    {
        TRACE = 0,
        DEBUG = 1,
        INFO  = 2,
        ERROR = 3
    };

    inline bool equalsIgnoreCase(std::string_view lhs, std::string_view rhs)
    {
        if (lhs.size() != rhs.size())
        {
            return false;
        }
        return std::equal(lhs.begin(), lhs.end(), rhs.begin(), [](char a, char b) {
            return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
        });
    }

    // Default logging is intentionally quiet; only errors are emitted unless overridden.
    inline LogLevel CURRENT_LOG_LEVEL = LogLevel::ERROR;

    inline LogLevel parseLogLevel(std::string_view value)
    {
        if (equalsIgnoreCase(value, "TRACE")) return LogLevel::TRACE;
        if (equalsIgnoreCase(value, "DEBUG")) return LogLevel::DEBUG;
        if (equalsIgnoreCase(value, "INFO"))  return LogLevel::INFO;
        return LogLevel::ERROR; // Fallback default
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

    inline LogLevel getConfiguredAampLogLevel()
    {
        const char *envValue = std::getenv("AAMP_LOG_LEVEL");
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
    constexpr std::string_view logLevelToString(LogLevel level)
    {
        switch (level)
        {
            case LogLevel::TRACE: return "TRACE";
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO:  return "INFO";
            case LogLevel::ERROR: return "ERROR";
        }
        return "UNKNOWN";
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
