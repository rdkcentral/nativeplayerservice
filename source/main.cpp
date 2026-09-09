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

#include "Application.h"
#include <chrono>
#include <iostream>
#include <csignal>
#include <thread>
#include "Logger.h"

volatile std::sig_atomic_t m_isActive = 1;
using namespace nativeplayer;
void waitForTermSignal()
{
    LOG(LogLevel::INFO, "Waiting for term signal.. ");

    while (m_isActive)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    LOG(LogLevel::INFO, "[waitForTermSignal] Received term signal.");
}

void handleTermSignal(int)
{
    m_isActive = 0;
}

int main(int argc, char *argv[])
{
    LOG(LogLevel::INFO, "Native Player 1.0");
    signal(SIGTERM, [](int x)
           { handleTermSignal(x); });
    signal(SIGINT, [](int x)
           { handleTermSignal(x); });

    nativeplayer::Application application;
    application.run();
    LOG(LogLevel::INFO, "Native Player is running.");
    waitForTermSignal();

    LOG(LogLevel::INFO, "Exiting application.");
    return 0;
}
