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

#ifndef REFPLAYER_APP_ACTIONS_CONNECTOR_H
#define REFPLAYER_APP_ACTIONS_CONNECTOR_H
#include <optional>
#include <string>
#include <memory>
#include "ConnectorBase.h"
#include "PlayerCommands.h"


namespace refplayer
{
    class AppActionsConnector : public ConnectorBase, public PlayerCommands
    {
    public:
        AppActionsConnector(std::unique_ptr<PlayerDelegate> playerDelegate, std::unique_ptr<FireboltConnector> fireboltConnector)
            : ConnectorBase(std::move(playerDelegate), std::move(fireboltConnector))
        {
        }
        ~AppActionsConnector() = default;

    private:
        bool registerForIntents();
        bool unregisterForIntents();
        bool handleIntent(const std::string &intent, const std::optional<std::string> &source);

        void handlePlayerEvent(const std::string &event);

        int onInitialize() override;
        void onStart() override;
        void onShutdown() override;

        std::string intentHandler;
    };
} // namespace refplayer

#endif // REFPLAYER_APP_ACTIONS_CONNECTOR_H