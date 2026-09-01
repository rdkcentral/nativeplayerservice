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

#ifndef REFPLAYER_CONNECTORFACTORY_H
#define REFPLAYER_CONNECTORFACTORY_H

#include <memory>
#include "IConnector.h"
#include "PlayerDelegate.h"
#include "FireboltConnector.h"

#include "WebSocketConnector.h"
#include "AppActionsConnector.h"

namespace refplayer
{
    enum class ConnectorType
    {
        WEBSOCKET,
        APPACTIONS
    };
    class ConnectorFactory

    {
    public:
        static std::unique_ptr<IConnector> create(ConnectorType type)
        {
            auto playerDelegate = std::make_unique<PlayerDelegate>();
            auto fireboltConnector = std::make_unique<FireboltConnector>();
            switch (type)
            {
            case ConnectorType::WEBSOCKET:
                return std::make_unique<WebSocketConnector>(std::move(playerDelegate), std::move(fireboltConnector));
            case ConnectorType::APPACTIONS:
                return std::make_unique<AppActionsConnector>(std::move(playerDelegate), std::move(fireboltConnector));
            default:
                return nullptr;
            }
        }
    };

} // namespace refplayer

#endif // REFPLAYER_CONNECTORFACTORY_H