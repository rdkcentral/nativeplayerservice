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
#include "PlayerDelegate.h"
#include "Logger.h"
#include <cstdlib>
#include <json/json.h>
#include "PlayerUtils.h"
namespace nativeplayer
{

    void PlayerDelegate::setPlayerEventListener(std::unique_ptr<PlayerEventListener> playerEvent)
    {
        m_playerEvent = std::move(playerEvent);
    }

    void PlayerDelegate::handleOpenSession(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received openSession request: ", request);
        // Check whether the play instance is already created or not, if not create a new instance of the player
        if (!m_playerInstance)
        {
            m_playerInstance = NativePlayer::getInstance();
            if (m_playerInstance)
            {
                LOG(LogLevel::INFO, "Attaching AAMP event callback to RPC server.");
                m_playerInstance->setEventCallback(
                    [this](const std::string &eventName, const Json::Value &params)
                    {
                        if (m_playerEvent)
                        {
                            Json::Value eventParams = params;
                            eventParams["sessionId"] = m_activeSessionId;
                            LOG(LogLevel::INFO, "Emitting RPC event: ", eventName);
                            m_playerEvent->onEvent(eventName, m_activeSessionId, eventParams);
                        }
                        else
                        {
                            LOG(LogLevel::ERROR, "Event listener not registered, dropping event: ", eventName);
                        }
                    });
            }
        }
        // If there is an active sesion, we won't allow opening a new session until the current session is closed.
        if (!m_activeSessionId.empty())
        {
            response = "{\"status\": false, \"message\": \"A session is already active. Please close the current session before opening a new one.\"}";
            return;
        }
        // Let us check whether the parameters are valid or not, if valid then we can open the session and return the response
        Json::Value requestJson;
        if (convertRawStringToJson(request, requestJson))
        {
            if (!requestJson.isMember("instanceId") || !requestJson["instanceId"].isString())
            {
                response = "{\"status\": false, \"message\": \"Invalid or missing parameter 'instanceId'.\"}";
                return;
            }
            if (!requestJson.isMember("displayId") || !requestJson["displayId"].isString())
            {
                response = "{\"status\": false, \"message\": \"Invalid or missing parameter 'displayId'.\"}";
                return;
            }

            std::string displayId = requestJson["displayId"].asString();
            setenv("WAYLAND_DISPLAY", displayId.c_str(), 1);
            std::string instanceId = requestJson["instanceId"].asString();
            m_playerInstance->setInstanceId(instanceId);

            // Generate a new session ID and store it as the active session
            m_activeSessionId = generateSessionId();
            response = "{\"status\": true, \"sessionId\": \"" + m_activeSessionId + "\"}";
        }
        else
        {
            response = "{\"status\": false, \"message\": \"Failed to parse request JSON.\"}";
        }
    }

    void PlayerDelegate::handleStop(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received stop request: ", request);
        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            m_playerInstance->stop();
            response = "{\"status\": true, \"message\": \"Playback stopped successfully.\"}";
        }
    }
    void PlayerDelegate::handleGetSessionInfo(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getSessionInfo request: ", request);
        if (m_activeSessionId.empty())
        {
            response = "{\"status\": false, \"message\": \"No active session found.\"}";
            return;
        }
        response = "{\"status\": true, \"sessionId\": \"" + m_activeSessionId + "\"}";
    }

    void PlayerDelegate::handlePlay(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received play request: ", request);
        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            if (requestJson.isMember("url") && requestJson["url"].isString())
            {
                std::string url = requestJson["url"].asString();
                if (m_playerInstance->play(url))
                {
                    response = "{\"status\": true, \"message\": \"Content playback started.\"}";
                }
                else
                {
                    response = "{\"status\": false, \"message\": \"Failed to start content playback.\"}";
                }
            }
            else

            {
                response = "{\"status\": false, \"message\": \"Invalid or missing 'url' parameter.\"}";
            }
        }
    }
    void PlayerDelegate::handleCloseSession(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received closeSession request: ", request);

        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {

            m_playerInstance->stop();
            m_playerInstance->setEventCallback({});
            m_playerInstance = nullptr;
            // Reset the WAYLAND_DISPLAY environment variable
            unsetenv("WAYLAND_DISPLAY");
        }
    }

    void PlayerDelegate::handleSeek(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received seek request: ", request);
        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            if (!requestJson.isMember("position") || !requestJson["position"].isNumeric())
            {
                response = "{\"status\": false, \"message\": \"Invalid or missing 'position' parameter.\"}";
                return;
            }
            double position = requestJson["position"].asDouble();
            bool keepPaused = (requestJson.isMember("keepPaused") && requestJson["keepPaused"].isBool()) ? requestJson["keepPaused"].asBool() : false;
            response = m_playerInstance->seek(position, keepPaused) ? "{\"status\": true}" : "{\"status\": false}";
        }
    }

    void PlayerDelegate::handleSeekToLive(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received seekToLive request: ", request);
        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            bool keepPaused = (requestJson.isMember("keepPaused") && requestJson["keepPaused"].isBool()) ? requestJson["keepPaused"].asBool() : false;
            response = m_playerInstance->seekToLive(keepPaused) ? "{\"status\": true}" : "{\"status\": false}";
        }
    }

    void PlayerDelegate::handleSetRate(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received setRate request: ", request);

        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            if (requestJson.isMember("rate") && requestJson["rate"].isNumeric())
            {
                float rate = requestJson["rate"].asFloat();
                int overshootCorrection = (requestJson.isMember("overshootCorrection") && requestJson["overshootCorrection"].isNumeric()) ? requestJson["overshootCorrection"].asInt() : 0;
                response = m_playerInstance->setRate(rate, overshootCorrection) ? "{\"status\": true}" : "{\"status\": false}";
            }
            else
            {
                response = "{\"status\": false, \"message\": \"Invalid or missing 'rate' parameter.\"}";
            }
        }
    }

    void PlayerDelegate::handleSetPlaybackSpeed(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received setPlaybackSpeed request: ", request);
        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            if (requestJson.isMember("speed") && requestJson["speed"].isNumeric())
            {
                float speed = requestJson["speed"].asFloat();
                response = m_playerInstance->setPlaybackSpeed(speed) ? "{\"status\": true}" : "{\"status\": false}";
            }
            else
            {
                response = "{\"status\": false, \"message\": \"Invalid or missing 'speed' parameter.\"}";
            }
        }
    }
    bool PlayerDelegate::validateSession(const std::string &request, Json::Value &requestJson, std::string &response)
    {
        bool status = true;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"status\": false, \"message\": \"No active session found.\"}";
            status = false;
        }
        else if (!convertRawStringToJson(request, requestJson))
        {
            response = "{\"status\": false, \"message\": \"Failed to parse request JSON.\"}";
            status = false;
        }
        else if (!isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"status\": false, \"message\": \"Invalid or missing 'sessionId' parameter.\"}";
            status = false;
        }
        return status;
    }
    void PlayerDelegate::handlePauseAt(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received pauseAt request: ", request);
        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            if (requestJson.isMember("position") && requestJson["position"].isNumeric())
            {
                double position = requestJson["position"].asDouble();
                response = m_playerInstance->pauseAt(position) ? "{\"status\": true}" : "{\"status\": false}";
            }
            else
            {
                response = "{\"status\": false, \"message\": \"Invalid or missing 'position' parameter.\"}";
                return;
            }
        }
    }

    void PlayerDelegate::handleSetRateAndSeek(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received setRateAndSeek request: ", request);

        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {

            if (requestJson.isMember("rate") && requestJson["rate"].isNumeric() &&
                requestJson.isMember("position") && requestJson["position"].isNumeric())
            {
                int rate = requestJson["rate"].asInt();

                double position = requestJson["position"].asDouble();
                response = m_playerInstance->setRateAndSeek(rate, position) ? "{\"status\": true}" : "{\"status\": false}";
            }
            else
            {
                response = "{\"status\": false, \"message\": \"Invalid or missing 'rate' or 'position' parameter.\"}";
                return;
            }
        }
    }

    void PlayerDelegate::handleGetState(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getState request: ", request);
        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            std::string state = m_playerInstance->getState();
            response = "{\"status\": true, \"state\": \"" + state + "\"}";
        }
    }

    void PlayerDelegate::handleGetPlaybackPosition(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getPlaybackPosition request: ", request);
        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            double position = m_playerInstance->getPlaybackPosition();
            response = "{\"status\": true, \"position\": " + std::to_string(position) + "}";
        }
    }

    void PlayerDelegate::handleGetPlaybackDuration(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getPlaybackDuration request: ", request);
        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            double duration = m_playerInstance->getPlaybackDuration();
            response = "{\"status\": true, \"duration\": " + std::to_string(duration) + "}";
        }
    }

    void PlayerDelegate::handleGetPlaybackRate(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getPlaybackRate request: ", request);
        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            int rate = m_playerInstance->getPlaybackRate();
            response = "{\"status\": true, \"rate\": " + std::to_string(rate) + "}";
        }
    }

    // ---------- Playback State ----------

    void PlayerDelegate::handleIsLive(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received isLive request: ", request);
        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            bool status = m_playerInstance->isLive();
            response = status ? "{\"status\": true, \"isLive\": true}" : "{\"status\": true, \"isLive\": false}";
        }
    }

    // ---------- Video ----------

    void PlayerDelegate::handleSetVideoMute(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received setVideoMute request: ", request);
        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            if (requestJson.isMember("muted") && requestJson["muted"].isBool())
            {

                bool muted = requestJson["muted"].asBool();
                response = m_playerInstance->setVideoMute(muted) ? "{\"status\": true, \"muted\": true}" : "{\"status\": true, \"muted\": false}";
            }
            else
            {
                response = "{\"status\": false, \"message\": \"Invalid or missing 'muted' parameter.\"}";
            }
        }
    }

    void PlayerDelegate::handleGetVideoMute(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getVideoMute request: ", request);
        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            response = m_playerInstance->getVideoMute() ? "{\"status\": true,\"muted\": true}" : "{\"status\": true,\"muted\": false}";
        }
    }

    // ---------- Audio ----------

    void PlayerDelegate::handleSetAudioVolume(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received setAudioVolume request: ", request);
        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            if (requestJson.isMember("volume") && requestJson["volume"].isNumeric())
            {
                int volume = requestJson["volume"].asInt();
                response = m_playerInstance->setAudioVolume(volume) ? "{\"status\": true}" : "{\"status\": false, \"message\": \"Failed to set audio volume.\"}";
            }
            else
            {
                response = "{\"status\": false, \"message\": \"Invalid or missing 'volume' parameter.\"}";
            }
        }
    }

    void PlayerDelegate::handleGetAudioVolume(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getAudioVolume request: ", request);
        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            int volume = m_playerInstance->getAudioVolume();
            response = "{\"status\": true, \"volume\": " + std::to_string(volume) + "}";
        }
    }

    void PlayerDelegate::handleGetAudioLanguage(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getAudioLanguage request: ", request);
        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            std::string lang = m_playerInstance->getAudioLanguage();
            response = std::string("{\"status\": true, \"language\": ") + Json::valueToQuotedString(lang.c_str()) + "}";
        }
    }

    void PlayerDelegate::handleGetAvailableAudioTracks(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getAvailableAudioTracks request: ", request);
        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            bool allTracks = requestJson.isMember("allTracks") ? requestJson["allTracks"].asBool() : false;
            std::string tracks = m_playerInstance->getAvailableAudioTracks(allTracks);
            response = "{\"status\": true, \"tracks\": " + tracks + "}";
        }
    }

    void PlayerDelegate::handleSetAudioTrack(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received setAudioTrack request: ", request);
        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            if (requestJson.isMember("trackId") && requestJson["trackId"].isNumeric())
            {
                int trackId = requestJson["trackId"].asInt();
                response = m_playerInstance->setAudioTrack(trackId) ? "{\"status\": true}" : "{\"status\": false}";
            }
            else
            {
                response = "{\"status\": false, \"message\": \"Invalid or missing 'trackId' parameter.\"}";
            }
        }
    }

    void PlayerDelegate::handleGetAudioTrack(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getAudioTrack request: ", request);
        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {

            int trackId = m_playerInstance->getAudioTrack();
            response = "{\"status\": true, \"trackId\": " + std::to_string(trackId) + "}";
        }
    }

    void PlayerDelegate::handleGetAudioTrackInfo(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getAudioTrackInfo request: ", request);
        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            std::string trackInfo = m_playerInstance->getAudioTrackInfo();
            response = "{\"status\": true, \"trackInfo\": " + trackInfo + "}";
        }
    }

    // ---------- Subtitles ----------

    void PlayerDelegate::handleSetSubtitleMute(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received setSubtitleMute request: ", request);
        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            if (requestJson.isMember("muted") && requestJson["muted"].isBool())
            {
                bool muted = requestJson["muted"].asBool();
                response = m_playerInstance->setSubtitleMute(muted) ? "{\"status\": true}" : "{\"status\": false}";
            }
            else
            {
                response = "{\"status\": false, \"message\": \"Invalid or missing 'muted' parameter.\"}";
            }
        }
    }

    void PlayerDelegate::handleGetAvailableTextTracks(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getAvailableTextTracks request: ", request);
        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            bool allTracks = requestJson.isMember("allTracks") ? requestJson["allTracks"].asBool() : false;
            std::string tracks = m_playerInstance->getAvailableTextTracks(allTracks);
            response = "{\"status\": true, \"tracks\": " + tracks + "}";
        }
    }

    void PlayerDelegate::handleSetTextTrack(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received setTextTrack request: ", request);
        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            if (requestJson.isMember("trackId") && requestJson["trackId"].isNumeric())
            {
                int trackId = requestJson["trackId"].asInt();
                response = m_playerInstance->setTextTrack(trackId) ? "{\"status\": true}" : "{\"status\": false}";
            }
            else
            {
                response = "{\"status\": false, \"message\": \"Invalid or missing 'trackId' parameter.\"}";
            }
        }
    }

    void PlayerDelegate::handleGetTextTrack(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getTextTrack request: ", request);

        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            int trackId = m_playerInstance->getTextTrack();
            response = "{\"status\": true, \"trackId\": " + std::to_string(trackId) + "}";
        }
    }

    // ---------- Bitrate / ABR ----------

    void PlayerDelegate::handleGetVideoBitrate(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getVideoBitrate request: ", request);

        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            int64_t bitrate = m_playerInstance->getVideoBitrate();
            response = "{\"status\": true, \"bitrate\": " + std::to_string(bitrate) + "}";
        }
    }

    void PlayerDelegate::handleSetVideoBitrate(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received setVideoBitrate request: ", request);
        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            if (requestJson.isMember("bitrate") && requestJson["bitrate"].isNumeric())
            {
                int64_t bitrate = requestJson["bitrate"].asInt64();
                response = m_playerInstance->setVideoBitrate(bitrate) ? "{\"status\": true}" : "{\"status\": false}";
            }
            else
            {
                response = "{\"status\": false, \"message\": \"Invalid or missing 'bitrate' parameter.\"}";
            }
        }
    }

    void PlayerDelegate::handleGetVideoBitrates(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getVideoBitrates request: ", request);
        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            std::vector<int64_t> bitrates = m_playerInstance->getVideoBitrates();
            std::string arr = "[";
            for (size_t i = 0; i < bitrates.size(); i++)
            {
                if (i > 0)
                    arr += ",";
                arr += std::to_string(bitrates[i]);
            }
            arr += "]";
            response = "{\"status\": true, \"bitrates\": " + arr + "}";
        }
    }

    void PlayerDelegate::handleSetInitialBitrate(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received setInitialBitrate request: ", request);
        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            if (requestJson.isMember("bitrate") && requestJson["bitrate"].isNumeric())
            {
                int64_t bitrate = requestJson["bitrate"].asInt64();
                response = m_playerInstance->setInitialBitrate(bitrate) ? "{\"status\": true}" : "{\"status\": false}";
            }
            else
            {
                response = "{\"status\": false, \"message\": \"Invalid or missing 'bitrate' parameter.\"}";
            }
        }
    }

    void PlayerDelegate::handleGetInitialBitrate(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getInitialBitrate request: ", request);
        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            int64_t bitrate = m_playerInstance->getInitialBitrate();
            response = "{\"status\": true,\"bitrate\": " + std::to_string(bitrate) + "}";
        }
    }

    void PlayerDelegate::handleSetMinimumBitrate(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received setMinimumBitrate request: ", request);
        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            if (requestJson.isMember("bitrate") && requestJson["bitrate"].isNumeric())
            {

                int64_t bitrate = requestJson["bitrate"].asInt64();
                response = m_playerInstance->setMinimumBitrate(bitrate) ? "{\"status\": true}" : "{\"status\": false}";
            }
            else
            {
                response = "{\"status\": false, \"message\": \"Invalid or missing 'bitrate' parameter.\"}";
            }
        }
    }

    void PlayerDelegate::handleGetMinimumBitrate(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getMinimumBitrate request: ", request);
        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            int64_t bitrate = m_playerInstance->getMinimumBitrate();
            response = "{\"status\": true, \"bitrate\": " + std::to_string(bitrate) + "}";
        }
    }

    void PlayerDelegate::handleSetMaximumBitrate(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received setMaximumBitrate request: ", request);
        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            if (requestJson.isMember("bitrate") && requestJson["bitrate"].isNumeric())
            {
                int64_t bitrate = requestJson["bitrate"].asInt64();
                response = m_playerInstance->setMaximumBitrate(bitrate) ? "{\"status\": true}" : "{\"status\": false}";
            }
            else
            {
                response = "{\"status\": false, \"message\": \"Invalid or missing 'bitrate' parameter.\"}";
            }
        }
    }

    void PlayerDelegate::handleGetMaximumBitrate(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getMaximumBitrate request: ", request);
        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            int64_t bitrate = m_playerInstance->getMaximumBitrate();
            response = "{\"status\": true, \"bitrate\": " + std::to_string(bitrate) + "}";
        }
    }

    // ---------- DRM ----------

    void PlayerDelegate::handleSetLicenseServerURL(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received setLicenseServerURL request");
        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            if (requestJson.isMember("url") && requestJson["url"].isString())
            {
                std::string url = requestJson["url"].asString();
                response = m_playerInstance->setLicenseServerURL(url) ? "{\"status\": true}" : "{\"status\": false}";
            }
            else
            {
                response = "{\"status\": false, \"message\": \"Invalid or missing 'url' parameter.\"}";
            }
        }
    }

            std::string drm = m_playerInstance->getDRM();
            response = std::string("{\"status\": true, \"drm\": ") + Json::valueToQuotedString(drm.c_str()) + "}";
    }

    void PlayerDelegate::handleSetPreferredDRM(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received setPreferredDRM request: ", request);
        Json::Value requestJson;
        if (validateSession(request, requestJson, response))
        {
            if (requestJson.isMember("drmType") && requestJson["drmType"].isString())
            {

                std::string drmType = requestJson["drmType"].asString();
                response = m_playerInstance->setPreferredDRM(drmType) ? "{\"status\": true}" : "{\"status\": false}";
            }
            else
            {
                response = "{\"status\": false, \"message\": \"Invalid or missing 'drmType' parameter.\"}";
            }
        }
    }

    // ---------- Configuration ----------

    void PlayerDelegate::handleConfigureSession(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received configureSession request: ", request);

        Json::Value requestJson;
        std::string configStr;
        if (validateSession(request, requestJson, response))
        {

            if (requestJson.isMember("config") && requestJson["config"].isString())
            {
                configStr = requestJson["config"].asString();
                bool ok = m_playerInstance->configureSession(configStr);
                response = ok ? "{\"status\": true}" : "{\"status\": false}";
            }
            else
            {
                response = "{\"status\": false, \"message\": \"Invalid or missing 'config' parameter.\"}";
            }
        }
    }

    void PlayerDelegate::handleGetAAMPConfig(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getAAMPConfig request: ", request);
        Json::Value requestJson;

        if (validateSession(request, requestJson, response))
        {
            std::string configStr;
            configStr = m_playerInstance->getAAMPConfig();
            response = "{{\"status\": true, \"config\": " + configStr + "}}";
        }
    }

    void PlayerDelegate::handleSetAppName(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received setAppName request: ", request);
        Json::Value requestJson;

        if (validateSession(request, requestJson, response))
        {
            if (requestJson.isMember("name") && requestJson["name"].isString())
            {
                std::string name = requestJson["name"].asString();
                response = m_playerInstance->setAppName(name) ? "{\"status\": true}" : "{\"status\": false}";
            }
            else
            {
                response = "{\"status\": false, \"message\": \"Invalid or missing 'name' parameter.\"}";
            }
        }
    }

    void PlayerDelegate::handleSetPreferredLanguages(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received setPreferredLanguages request: ", request);
        Json::Value requestJson;

        if (validateSession(request, requestJson, response))
        {
            std::string languageList = (requestJson.isMember("languageList") && requestJson["languageList"].isString()) ? requestJson["languageList"].asString() : "";
            std::string rendition = (requestJson.isMember("rendition") && requestJson["rendition"].isString()) ? requestJson["rendition"].asString() : "";
            std::string type = (requestJson.isMember("type") && requestJson["type"].isString()) ? requestJson["type"].asString() : "";
            std::string codecList = (requestJson.isMember("codecList") && requestJson["codecList"].isString()) ? requestJson["codecList"].asString() : "";
            std::string labelList = (requestJson.isMember("labelList") && requestJson["labelList"].isString()) ? requestJson["labelList"].asString() : "";
            bool ok = m_playerInstance->setPreferredLanguages(languageList, rendition, type, codecList, labelList);
            response = ok ? "{\"status\": true}" : "{\"status\": false}";
        }
    }

    void PlayerDelegate::handleGetPreferredLanguages(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getPreferredLanguages request: ", request);
        Json::Value requestJson;

        if (validateSession(request, requestJson, response))
        {
            std::string langList = m_playerInstance->getPreferredLanguages();
            response = std::string("{\"status\": true, \"languageList\": ") + Json::valueToQuotedString(langList.c_str()) + "}";
        }
    }
} // namespace nativeplayer