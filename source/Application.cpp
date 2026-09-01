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
#include <cstdlib> //For getEnv
#include <iostream>
#include "Logger.h"
#include "PlayerEvent.h"
#include "PlayerDelegate.h"
#include "ConnectorFactory.h"

namespace refplayer
{

    int Application::run()
    {
        LOG(LogLevel::INFO, "Starting ", title());
        if (m_Connector->initialize() != 0)
        {
            LOG(LogLevel::ERROR, "Failed to initialize IConnector");
            return -1;
        }
        m_Connector->start();
        // The application would typically enter its main loop here
        // For demonstration purposes, we'll just wait for user input to exit
        return 0;
    }

    Application::Application()
    {
        // Create a unique instance of IConnector using the factory
        m_Connector = ConnectorFactory::create(ConnectorType::WEBSOCKET); // Initialize the IConnector using the factory
        // Constructor implementation
    }
    Application::~Application()
    {
        if (m_Connector)
        {
            m_Connector->shutdown(); // Ensure the connector is properly shut down
        }
        // Destructor implementation
    }

    std::string Application::title() const
    {
        return "RefPlayer 1.0";
    }

} // namespace refplayer
