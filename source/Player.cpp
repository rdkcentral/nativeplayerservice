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

#include "Player.h"
#include <glib.h>
// Header for gst_init
#include <gst/gst.h>
#include <iostream>
#include <sstream>
#include <vector>
#include "AampLogManager.h"
#include "PlayerUtils.h"
#include "Logger.h"
// AAMP event types
#include <AampEvent.h>
namespace refplayer
{
    RefPlayer *RefPlayer::m_instance = nullptr;

    RefPlayer *RefPlayer::getInstance()
    {
        if (!m_instance)
        {
            m_instance = new RefPlayer();
            LOG(LogLevel::INFO, "RefPlayer instance created.");
            if (m_instance->initializePlayer())
            {
                LOG(LogLevel::INFO, "Player initialized successfully.");
                // Initialization successful
            }
            else
            {
                // Initialization failed, handle error
                LOG(LogLevel::ERROR, "Failed to initialize Player.");
                delete m_instance;
                m_instance = nullptr;
            }
        }
        return m_instance;
    }

    void RefPlayer::shutdownPlayer()
    {
        if (m_player)
        {
            delete m_player;
            m_player = nullptr;
        }
        if (m_eventListener)
        {
            delete m_eventListener;
            m_eventListener = nullptr;
        }
        if (m_eventLoop)
        {
            g_main_loop_quit(m_eventLoop);
            g_main_loop_unref(m_eventLoop);
            m_eventLoop = nullptr;
        }
        if (m_eventThread)
        {
            g_thread_join(m_eventThread);
            m_eventThread = nullptr;
        }
    }
    RefPlayer::RefPlayer()
        : m_playerReady(false),
          m_player(nullptr),
          m_eventListener(nullptr)
    {
    }

    gpointer RefPlayer::RefPlayerStreamThread(gpointer arg)
    {
        // Thread implementation for AAMP GStreamer player stream
        m_eventLoop = g_main_loop_new(nullptr, FALSE);
        g_main_loop_run(m_eventLoop); // Blocking call to run the main loop
        LOG(LogLevel::INFO, "Exiting AAMP GStreamer player stream thread.");
        g_main_loop_unref(m_eventLoop);
        m_eventLoop = nullptr;
        return nullptr;
    }

    bool RefPlayer::initializePlayer()
    {
        // Initialize the gstreamer player instancurle
        gst_init(nullptr, nullptr);

        if (gst_debug_is_active())
        {
            g_print("GStreamer Debug Engine is: ENABLED\n");
        }
        else
        {
            g_print("GStreamer Debug Engine is: DISABLED (Stripped at compilation)\n");
        }

        m_eventThread = g_thread_new("RefPlayerStreamThread", [](gpointer arg) -> gpointer
                                     { return static_cast<RefPlayer *>(arg)->RefPlayerStreamThread(arg); }, this);

        // Keep full AAMP verbosity for troubleshooting.
        AampLogManager::lockLogLevel(false);
        AampLogManager::setLogLevel(eLOGLEVEL_TRACE);
        AampLogManager::lockLogLevel(true);

        // Start the main loop for GStreamer

        m_player = new PlayerInstanceAAMP();

        if (m_player)
        {

            // string config
            m_player->mConfig.SetConfigValue(
                AAMP_APPLICATION_SETTING,
                eAAMPConfig_UserAgent,
                std::string("RefPlayer/1.0"));
            // Register event listener
            m_eventListener = new RefPlayerEventListener();
            m_player->RegisterEvents(m_eventListener);

            m_playerReady = true;
        }
        else
        {
            m_playerReady = false;
        }

        return m_playerReady;
    }
    RefPlayer::~RefPlayer()
    {
        // Destructor implementation

        if (m_player && m_eventListener)
        {
            m_player->UnRegisterEvents(m_eventListener);
        }
        if (m_eventListener)
        {
            delete m_eventListener;
            m_eventListener = nullptr;
        }
        if (m_player)
        {
            delete m_player;
            m_player = nullptr;
        }
    }

    void RefPlayer::setInstanceId(const std::string &instanceId)
    {
        // Set instance ID implementation
        m_player->SetAppName(instanceId.c_str());
    }

    bool RefPlayer::isPlaying() const
    {
        // Check if playing implementation
        return false;
    }

    bool RefPlayer::play(const std::string &url)
    {
        // locator,autoplay,contentType,firstAttempt,finalAttempt,traceUUID,audioDecoderStreamSync
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return false;
        }
        if (url.empty())
        {
            LOG(LogLevel::ERROR, "Invalid URL.");
            return false;
        }
        LOG(LogLevel::INFO, "Starting playback for URL: ", url);
        m_player->Tune(url.c_str(), true, nullptr, true, false, nullptr, true);

        return true;
    }

    bool RefPlayer::stop()
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return false;
        }
        LOG(LogLevel::INFO, "Stopping playback.");
        m_player->Stop();
        return true;
    }

    bool RefPlayer::pause()
    {
        // Pause implementation
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return false;
        }
        LOG(LogLevel::INFO, "Pausing playback.");
        m_player->PauseAt(0.0);
        return true;
    }

    bool RefPlayer::resume()
    {
        // Resume implementation
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return false;
        }
        LOG(LogLevel::INFO, "Resuming playback.");
        m_player->SetRate(1);
        return true;
    }

    bool RefPlayer::isPaused() const
    {
        // Check if paused implementation
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return false;
        }
        return m_player->GetPlaybackRate() == 0.0;
    }

    bool RefPlayer::seek(double position, bool keepPaused)
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return false;
        }
        LOG(LogLevel::INFO, "Seeking to position: ", position, " keepPaused: ", keepPaused);
        m_player->Seek(position, keepPaused);
        return true;
    }

    bool RefPlayer::seekToLive(bool keepPaused)
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return false;
        }
        LOG(LogLevel::INFO, "Seeking to live edge. keepPaused: ", keepPaused);
        m_player->SeekToLive(keepPaused);
        return true;
    }

    bool RefPlayer::setRate(float rate, int overshootCorrection)
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return false;
        }
        LOG(LogLevel::INFO, "Setting playback rate: ", rate, " overshootCorrection: ", overshootCorrection);
        m_player->SetRate(rate, overshootCorrection);
        return true;
    }

    bool RefPlayer::setPlaybackSpeed(float speed)
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return false;
        }
        LOG(LogLevel::INFO, "Setting playback speed: ", speed);
        m_player->SetRate(speed);
        return true;
    }

    bool RefPlayer::pauseAt(double position)
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return false;
        }
        LOG(LogLevel::INFO, "Scheduling pause at position: ", position);
        m_player->PauseAt(position);
        return true;
    }

    bool RefPlayer::setRateAndSeek(int rate, double position)
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return false;
        }
        LOG(LogLevel::INFO, "Setting rate: ", rate, " and seeking to: ", position);
        m_player->SetRateAndSeek(rate, position);
        return true;
    }

    std::string RefPlayer::getState()
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return "idle";
        }
        AAMPPlayerState state = m_player->GetState();
        switch (state)
        {
        case eSTATE_IDLE:
            return "idle";
        case eSTATE_INITIALIZING:
            return "initializing";
        case eSTATE_INITIALIZED:
            return "initialized";
        case eSTATE_PREPARING:
            return "preparing";
        case eSTATE_PREPARED:
            return "prepared";
        case eSTATE_BUFFERING:
            return "buffering";
        case eSTATE_PAUSED:
            return "paused";
        case eSTATE_SEEKING:
            return "seeking";
        case eSTATE_PLAYING:
            return "playing";
        case eSTATE_STOPPING:
            return "stopping";
        case eSTATE_STOPPED:
            return "stopped";
        case eSTATE_COMPLETE:
            return "complete";
        case eSTATE_ERROR:
            return "error";
        case eSTATE_RELEASED:
            return "released";
        case eSTATE_BLOCKED:
            return "blocked";
        default:
            return "idle";
        }
    }

    double RefPlayer::getPlaybackPosition()
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return 0.0;
        }
        return m_player->GetPlaybackPosition();
    }

    double RefPlayer::getPlaybackDuration()
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return -1.0;
        }
        return m_player->GetPlaybackDuration();
    }

    int RefPlayer::getPlaybackRate()
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return 0;
        }
        return m_player->GetPlaybackRate();
    }

    bool RefPlayer::isLive()
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return false;
        }
        return m_player->IsLive();
    }

    bool RefPlayer::setVideoMute(bool muted)
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return false;
        }
        LOG(LogLevel::INFO, "Setting video mute: ", muted);
        m_player->SetVideoMute(muted);
        return true;
    }

    bool RefPlayer::getVideoMute()
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return false;
        }
        return m_player->GetVideoMute();
    }

    bool RefPlayer::setAudioVolume(int volume)
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return false;
        }
        LOG(LogLevel::INFO, "Setting audio volume: ", volume);
        m_player->SetAudioVolume(volume);
        return true;
    }

    int RefPlayer::getAudioVolume()
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return 0;
        }
        return m_player->GetAudioVolume();
    }

    std::string RefPlayer::getAudioLanguage()
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return "";
        }
        return m_player->GetAudioLanguage();
    }

    std::string RefPlayer::getAvailableAudioTracks(bool allTracks)
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return "[]";
        }
        return m_player->GetAvailableAudioTracks(allTracks);
    }

    bool RefPlayer::setAudioTrack(int trackId)
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return false;
        }
        LOG(LogLevel::INFO, "Setting audio track: ", trackId);
        m_player->SetAudioTrack(trackId);
        return true;
    }

    int RefPlayer::getAudioTrack()
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return -1;
        }
        return m_player->GetAudioTrack();
    }

    std::string RefPlayer::getAudioTrackInfo()
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return "{}";
        }
        return m_player->GetAudioTrackInfo();
    }

    bool RefPlayer::setSubtitleMute(bool muted)
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return false;
        }
        LOG(LogLevel::INFO, "Setting subtitle mute: ", muted);
        m_player->SetSubtitleMute(muted);
        return true;
    }

    std::string RefPlayer::getAvailableTextTracks(bool allTracks)
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return "[]";
        }
        return m_player->GetAvailableTextTracks(allTracks);
    }

    bool RefPlayer::setTextTrack(int trackId)
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return false;
        }
        LOG(LogLevel::INFO, "Setting text track: ", trackId);
        m_player->SetTextTrack(trackId);
        return true;
    }

    int RefPlayer::getTextTrack()
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return -1;
        }
        return m_player->GetTextTrack();
    }

    int64_t RefPlayer::getVideoBitrate()
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return 0;
        }
        return static_cast<int64_t>(m_player->GetVideoBitrate());
    }

    bool RefPlayer::setVideoBitrate(int64_t bitrate)
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return false;
        }
        LOG(LogLevel::INFO, "Setting video bitrate: ", bitrate);
        m_player->SetVideoBitrate(static_cast<BitsPerSecond>(bitrate));
        return true;
    }

    std::vector<int64_t> RefPlayer::getVideoBitrates()
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return {};
        }
        std::vector<BitsPerSecond> aampBitrates = m_player->GetVideoBitrates();
        std::vector<int64_t> bitrates(aampBitrates.begin(), aampBitrates.end());
        return bitrates;
    }

    bool RefPlayer::setInitialBitrate(int64_t bitrate)
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return false;
        }
        LOG(LogLevel::INFO, "Setting initial bitrate: ", bitrate);
        m_player->SetInitialBitrate(static_cast<BitsPerSecond>(bitrate));
        return true;
    }

    int64_t RefPlayer::getInitialBitrate()
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return 0;
        }
        return static_cast<int64_t>(m_player->GetInitialBitrate());
    }

    bool RefPlayer::setMinimumBitrate(int64_t bitrate)
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return false;
        }
        LOG(LogLevel::INFO, "Setting minimum bitrate: ", bitrate);
        m_player->SetMinimumBitrate(static_cast<BitsPerSecond>(bitrate));
        return true;
    }

    int64_t RefPlayer::getMinimumBitrate()
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return 0;
        }
        return static_cast<int64_t>(m_player->GetMinimumBitrate());
    }

    bool RefPlayer::setMaximumBitrate(int64_t bitrate)
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return false;
        }
        LOG(LogLevel::INFO, "Setting maximum bitrate: ", bitrate);
        m_player->SetMaximumBitrate(static_cast<BitsPerSecond>(bitrate));
        return true;
    }

    int64_t RefPlayer::getMaximumBitrate()
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return 0;
        }
        return static_cast<int64_t>(m_player->GetMaximumBitrate());
    }

    bool RefPlayer::setLicenseServerURL(const std::string &url)
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return false;
        }
        LOG(LogLevel::INFO, "Setting license server URL");
        m_player->SetLicenseServerURL(url.c_str());
        return true;
    }

    std::string RefPlayer::getDRM()
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return "none";
        }
        return m_player->GetDRM();
    }

    bool RefPlayer::setPreferredDRM(const std::string &drmType)
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return false;
        }
        DRMSystems drm = eDRM_MAX_DRMSystems;
        if (drmType == "widevine")
            drm = eDRM_WideVine;
        else if (drmType == "playready")
            drm = eDRM_PlayReady;
        else if (drmType == "clearkey")
            drm = eDRM_ClearKey;
        else
        {
            LOG(LogLevel::ERROR, "Unknown DRM type: ", drmType);
            return false;
        }
        LOG(LogLevel::INFO, "Setting preferred DRM: ", drmType);
        m_player->SetPreferredDRM(drm);
        return true;
    }

    bool RefPlayer::configureSession(const std::string &configJson)
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return false;
        }
        LOG(LogLevel::INFO, "Applying AAMP config.");
        return m_player->InitAAMPConfig(configJson.c_str());
    }

    std::string RefPlayer::getAAMPConfig()
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return "{}";
        }
        return m_player->GetAAMPConfig();
    }

    bool RefPlayer::setAppName(const std::string &name)
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return false;
        }
        LOG(LogLevel::INFO, "Setting app name: ", name);
        m_player->SetAppName(name);
        return true;
    }

    bool RefPlayer::setPreferredLanguages(const std::string &languageList,
                                                  const std::string &rendition,
                                                  const std::string &type,
                                                  const std::string &codecList,
                                                  const std::string &labelList)
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return false;
        }
        LOG(LogLevel::INFO, "Setting preferred languages: ", languageList);
        m_player->SetPreferredLanguages(
            languageList.empty() ? nullptr : languageList.c_str(),
            rendition.empty() ? nullptr : rendition.c_str(),
            type.empty() ? nullptr : type.c_str(),
            codecList.empty() ? nullptr : codecList.c_str(),
            labelList.empty() ? nullptr : labelList.c_str());
        return true;
    }

    std::string RefPlayer::getPreferredLanguages()
    {
        if (!m_playerReady || !m_player)
        {
            LOG(LogLevel::ERROR, "Player not initialized.");
            return "";
        }
        return m_player->GetPreferredLanguages();
    }

    void RefPlayer::setEventCallback(RefPlayerEventListener::EventCallback cb)
    {
        if (m_eventListener)
        {
            LOG(LogLevel::INFO, "Registering AAMP event callback on PlayerEventListener.");
            m_eventListener->setEventCallback(std::move(cb));
        }
        else
        {
            LOG(LogLevel::ERROR, "Cannot set event callback: PlayerEventListener is null.");
        }
    }

    // Event listener overrides

    const char *RefPlayerEventListener::stringifyPlayerState(AAMPPlayerState state)
    {
        return "";
    }
    void RefPlayerEventListener::Event(const AAMPEventPtr &e)
    {
        AAMPEventType type = e->getType();
        LOG(LogLevel::INFO, "Received AAMP event: ", mapAAMPEventToString(type));

        RefPlayerEventListener::EventCallback cb;
        {
            std::lock_guard<std::mutex> lock(m_eventCallbackMutex);
            cb = m_eventCallback;
        }
        if (!cb)
        {
            LOG(LogLevel::TRACE, "No event callback registered, dropping AAMP event: ", mapAAMPEventToString(type));
            return;
        }

        std::string eventName;
        Json::Value params;

        switch (type)
        {
        case AAMP_EVENT_TUNED:
        {
            eventName = "onTuned";
            LOG(LogLevel::INFO, "AAMP_EVENT_TUNED ");
            break;
        }
        case AAMP_EVENT_TUNE_FAILED:
        {
            auto ev = std::dynamic_pointer_cast<MediaErrorEvent>(e);
            eventName = "onTuneFailed";
            if (ev)
            {
                params["description"] = ev->getDescription();
                params["code"] = ev->getCode();
                params["shouldRetry"] = ev->shouldRetry();
                LOG(LogLevel::ERROR, "AAMP_EVENT_TUNE_FAILED: code=", ev->getCode(), " description=", ev->getDescription());
            }
            break;
        }
        case AAMP_EVENT_STATE_CHANGED:
        {
            auto ev = std::dynamic_pointer_cast<StateChangedEvent>(e);
            eventName = "onStateChanged";
            if (ev)
            {
                const char *stateStr = "idle";
                switch (ev->getState())
                {
                case eSTATE_IDLE:
                    stateStr = "idle";
                    break;
                case eSTATE_INITIALIZING:
                    stateStr = "initializing";
                    break;
                case eSTATE_INITIALIZED:
                    stateStr = "initialized";
                    break;
                case eSTATE_PREPARING:
                    stateStr = "preparing";
                    break;
                case eSTATE_PREPARED:
                    stateStr = "prepared";
                    break;
                case eSTATE_BUFFERING:
                    stateStr = "buffering";
                    break;
                case eSTATE_PAUSED:
                    stateStr = "paused";
                    break;
                case eSTATE_SEEKING:
                    stateStr = "seeking";
                    break;
                case eSTATE_PLAYING:
                    stateStr = "playing";
                    break;
                case eSTATE_STOPPING:
                    stateStr = "stopping";
                    break;
                case eSTATE_STOPPED:
                    stateStr = "stopped";
                    break;
                case eSTATE_COMPLETE:
                    stateStr = "complete";
                    break;
                case eSTATE_ERROR:
                    stateStr = "error";
                    break;
                case eSTATE_RELEASED:
                    stateStr = "released";
                    break;
                case eSTATE_BLOCKED:
                    stateStr = "blocked";
                    break;
                default:
                    stateStr = "idle";
                    break;
                }
                params["state"] = stateStr;
                LOG(LogLevel::INFO, "AAMP_EVENT_STATE_CHANGED: state=", stateStr);
            }
            break;
        }
        case AAMP_EVENT_PROGRESS:
        {
            auto ev = std::dynamic_pointer_cast<ProgressEvent>(e);
            eventName = "onProgress";
            if (ev)
            {
                params["positionMs"] = ev->getPosition();
                params["durationMs"] = ev->getDuration();
                params["speed"] = ev->getSpeed();
                params["startMs"] = ev->getStart();
                params["endMs"] = ev->getEnd();
                params["videoBufferedMs"] = ev->getVideoBufferedDuration();
                params["audioBufferedMs"] = ev->getAudioBufferedDuration();
                params["liveLatencyMs"] = ev->getLiveLatency();
                params["profileBitrate"] = static_cast<Json::Int64>(ev->getProfileBandwidth());
                params["networkBitrate"] = static_cast<Json::Int64>(ev->getNetworkBandwidth());
                LOG(LogLevel::TRACE, "AAMP_EVENT_PROGRESS: position=", ev->getPosition(), " duration=", ev->getDuration());
            }
            break;
        }
        case AAMP_EVENT_EOS:
        {
            eventName = "onEOS";
            LOG(LogLevel::INFO, "AAMP_EVENT_EOS: end of stream reached.");
            break;
        }
        case AAMP_EVENT_SPEED_CHANGED:
        {
            auto ev = std::dynamic_pointer_cast<SpeedChangedEvent>(e);
            eventName = "onSpeedChanged";
            if (ev)
            {
                params["speed"] = ev->getRate();
                LOG(LogLevel::INFO, "AAMP_EVENT_SPEED_CHANGED: speed=", ev->getRate());
            }
            break;
        }
        case AAMP_EVENT_BUFFERING_CHANGED:
        {
            auto ev = std::dynamic_pointer_cast<BufferingChangedEvent>(e);
            eventName = "onBufferingChanged";
            if (ev)
            {
                params["buffering"] = ev->buffering();
                LOG(LogLevel::INFO, "AAMP_EVENT_BUFFERING_CHANGED: buffering=", ev->buffering());
            }
            break;
        }
        case AAMP_EVENT_SEEKED:
        {
            auto ev = std::dynamic_pointer_cast<SeekedEvent>(e);
            eventName = "onSeeked";
            if (ev)
            {
                params["positionMs"] = ev->getPosition();
                LOG(LogLevel::INFO, "AAMP_EVENT_SEEKED: position=", ev->getPosition());
            }
            break;
        }
        case AAMP_EVENT_BITRATE_CHANGED:
        {
            auto ev = std::dynamic_pointer_cast<BitrateChangeEvent>(e);
            eventName = "onBitrateChanged";
            if (ev)
            {
                params["bitrate"] = static_cast<Json::Int64>(ev->getBitrate());
                params["description"] = ev->getDescription();
                params["width"] = ev->getWidth();
                params["height"] = ev->getHeight();
                params["frameRate"] = ev->getFrameRate();
                params["position"] = ev->getPosition();
                params["cappedProfile"] = ev->getCappedProfileStatus();
                params["displayWidth"] = ev->getDisplayWidth();
                params["displayHeight"] = ev->getDisplayHeight();
                params["videoScanType"] = ev->getScanType();
                params["aspectRatioWidth"] = ev->getAspectRatioWidth();
                params["aspectRatioHeight"] = ev->getAspectRatioHeight();
                LOG(LogLevel::INFO, "AAMP_EVENT_BITRATE_CHANGED: bitrate=", ev->getBitrate(), " ", ev->getWidth(), "x", ev->getHeight());
            }
            break;
        }
        case AAMP_EVENT_CC_HANDLE_RECEIVED:
        {
            auto ev = std::dynamic_pointer_cast<CCHandleEvent>(e);
            eventName = "onCCHandleReceived";
            if (ev)
            {
                params["handle"] = static_cast<Json::UInt64>(ev->getCCHandle());
                LOG(LogLevel::INFO, "AAMP_EVENT_CC_HANDLE_RECEIVED");
            }
            break;
        }
        case AAMP_EVENT_MEDIA_METADATA:
        {
            auto ev = std::dynamic_pointer_cast<MediaMetadataEvent>(e);
            eventName = "onMediaMetadata";
            if (ev)
            {
                params["durationMs"] = static_cast<Json::Int64>(ev->getDuration());
                params["width"] = ev->getWidth();
                params["height"] = ev->getHeight();
                params["hasDrm"] = ev->hasDrm();
                params["programStartTime"] = ev->getProgramStartTime();
                params["tsbDepthMs"] = ev->getTsbDepth();
                Json::Value languages(Json::arrayValue);
                for (const auto &lang : ev->getLanguages())
                    languages.append(lang);
                params["languages"] = languages;
                Json::Value bitrates(Json::arrayValue);
                for (auto br : ev->getBitrates())
                    bitrates.append(static_cast<Json::Int64>(br));
                params["bitrates"] = bitrates;
                Json::Value speeds(Json::arrayValue);
                for (auto sp : ev->getSupportedSpeeds())
                    speeds.append(sp);
                params["supportedSpeeds"] = speeds;
                LOG(LogLevel::INFO, "AAMP_EVENT_MEDIA_METADATA: ", ev->getWidth(), "x", ev->getHeight(), " durationMs=", ev->getDuration());
            }
            break;
        }
        case AAMP_EVENT_TIMED_METADATA:
        {
            auto ev = std::dynamic_pointer_cast<TimedMetadataEvent>(e);
            eventName = "onTimedMetadata";
            if (ev)
            {
                params["name"] = ev->getName();
                params["id"] = ev->getId();
                params["timeMs"] = ev->getTime();
                params["durationMs"] = ev->getDuration();
                params["content"] = ev->getContent();
                LOG(LogLevel::INFO, "AAMP_EVENT_TIMED_METADATA: name=", ev->getName());
            }
            break;
        }
        case AAMP_EVENT_BULK_TIMED_METADATA:
        {
            auto ev = std::dynamic_pointer_cast<BulkTimedMetadataEvent>(e);
            eventName = "onBulkTimedMetadata";
            if (ev)
            {
                params["content"] = ev->getContent();
                LOG(LogLevel::INFO, "AAMP_EVENT_BULK_TIMED_METADATA");
            }
            break;
        }
        case AAMP_EVENT_SPEEDS_CHANGED:
        {
            auto ev = std::dynamic_pointer_cast<SupportedSpeedsChangedEvent>(e);
            eventName = "onSpeedsChanged";
            if (ev)
            {
                Json::Value speeds(Json::arrayValue);
                for (auto sp : ev->getSupportedSpeeds())
                    speeds.append(sp);
                params["supportedSpeeds"] = speeds;
                LOG(LogLevel::INFO, "AAMP_EVENT_SPEEDS_CHANGED: count=", ev->getSupportedSpeedCount());
            }
            break;
        }
        case AAMP_EVENT_TUNE_PROFILING:
        {
            auto ev = std::dynamic_pointer_cast<TuneProfilingEvent>(e);
            eventName = "onTuneProfiling";
            if (ev)
            {
                params["profilingData"] = ev->getProfilingData();
                LOG(LogLevel::INFO, "AAMP_EVENT_TUNE_PROFILING");
            }
            break;
        }
        case AAMP_EVENT_DRM_METADATA:
        {
            auto ev = std::dynamic_pointer_cast<DrmMetaDataEvent>(e);
            eventName = "onDrmMetadata";
            if (ev)
            {
                params["failure"] = ev->getFailure();
                params["accessStatus"] = ev->getAccessStatus();
                params["accessStatusValue"] = ev->getAccessStatusValue();
                params["responseCode"] = ev->getResponseCode();
                params["isSecClientError"] = ev->getSecclientError();
                LOG(LogLevel::INFO, "AAMP_EVENT_DRM_METADATA: failure=", ev->getFailure(), " responseCode=", ev->getResponseCode());
            }
            break;
        }
        case AAMP_EVENT_REPORT_ANOMALY:
        {
            auto ev = std::dynamic_pointer_cast<AnomalyReportEvent>(e);
            eventName = "onAnomalyReport";
            if (ev)
            {
                params["severity"] = ev->getSeverity();
                params["message"] = ev->getMessage();
                LOG(LogLevel::INFO, "AAMP_EVENT_REPORT_ANOMALY: severity=", ev->getSeverity(), " msg=", ev->getMessage());
            }
            break;
        }
        case AAMP_EVENT_WEBVTT_CUE_DATA:
        {
            eventName = "onWebVttCueData";
            LOG(LogLevel::INFO, "AAMP_EVENT_WEBVTT_CUE_DATA");
            break;
        }
        case AAMP_EVENT_AD_RESOLVED:
        {
            auto ev = std::dynamic_pointer_cast<AdResolvedEvent>(e);
            eventName = "onAdResolved";
            if (ev)
            {
                params["resolveStatus"] = ev->getResolveStatus();
                params["adId"] = ev->getAdId();
                params["startMs"] = ev->getStart();
                params["durationMs"] = ev->getDuration();
                params["errorCode"] = ev->getErrorCode();
                params["errorDescription"] = ev->getErrorDescription();
                LOG(LogLevel::INFO, "AAMP_EVENT_AD_RESOLVED: adId=", ev->getAdId(), " resolved=", ev->getResolveStatus());
            }
            break;
        }
        case AAMP_EVENT_AD_RESERVATION_START:
        case AAMP_EVENT_AD_RESERVATION_END:
        {
            auto ev = std::dynamic_pointer_cast<AdReservationEvent>(e);
            eventName = (type == AAMP_EVENT_AD_RESERVATION_START) ? "onAdReservationStart" : "onAdReservationEnd";
            if (ev)
            {
                params["adBreakId"] = ev->getAdBreakId();
                params["position"] = ev->getPosition();
                LOG(LogLevel::INFO, eventName, ": adBreakId=", ev->getAdBreakId());
            }
            break;
        }
        case AAMP_EVENT_AD_PLACEMENT_START:
        case AAMP_EVENT_AD_PLACEMENT_END:
        case AAMP_EVENT_AD_PLACEMENT_ERROR:
        case AAMP_EVENT_AD_PLACEMENT_PROGRESS:
        {
            auto ev = std::dynamic_pointer_cast<AdPlacementEvent>(e);
            if (type == AAMP_EVENT_AD_PLACEMENT_START)
                eventName = "onAdPlacementStart";
            else if (type == AAMP_EVENT_AD_PLACEMENT_END)
                eventName = "onAdPlacementEnd";
            else if (type == AAMP_EVENT_AD_PLACEMENT_ERROR)
                eventName = "onAdPlacementError";
            else
                eventName = "onAdPlacementProgress";
            if (ev)
            {
                params["adId"] = ev->getAdId();
                params["position"] = ev->getPosition();
                params["offset"] = ev->getOffset();
                params["durationMs"] = ev->getDuration();
                params["errorCode"] = ev->getErrorCode();
                LOG(LogLevel::INFO, eventName, ": adId=", ev->getAdId());
            }
            break;
        }
        case AAMP_EVENT_REPORT_METRICS_DATA:
        {
            auto ev = std::dynamic_pointer_cast<MetricsDataEvent>(e);
            eventName = "onMetricsData";
            if (ev)
            {
                params["type"] = ev->getMetricsDataType();
                params["metricUUID"] = ev->getMetricUUID();
                params["data"] = ev->getMetricsData();
                LOG(LogLevel::INFO, "AAMP_EVENT_REPORT_METRICS_DATA: type=", ev->getMetricsDataType());
            }
            break;
        }
        case AAMP_EVENT_ID3_METADATA:
        {
            auto ev = std::dynamic_pointer_cast<ID3MetadataEvent>(e);
            eventName = "onID3Metadata";
            if (ev)
            {
                params["length"] = static_cast<Json::UInt>(ev->getMetadataSize());
                params["schemeIdUri"] = ev->getSchemeIdUri();
                params["id3Value"] = ev->getValue();
                params["presentationTime"] = static_cast<Json::UInt64>(ev->getPresentationTime());
                params["timeScale"] = static_cast<Json::UInt>(ev->getTimeScale());
                LOG(LogLevel::INFO, "AAMP_EVENT_ID3_METADATA: length=", ev->getMetadataSize());
            }
            break;
        }
        case AAMP_EVENT_DRM_MESSAGE:
        {
            auto ev = std::dynamic_pointer_cast<DrmMessageEvent>(e);
            eventName = "onDrmMessage";
            if (ev)
            {
                params["message"] = ev->getMessage();
                LOG(LogLevel::INFO, "AAMP_EVENT_DRM_MESSAGE");
            }
            break;
        }
        case AAMP_EVENT_CONTENT_GAP:
        {
            auto ev = std::dynamic_pointer_cast<ContentGapEvent>(e);
            eventName = "onContentGap";
            if (ev)
            {
                params["timeMs"] = ev->getTime();
                params["durationMs"] = ev->getDuration();
                LOG(LogLevel::INFO, "AAMP_EVENT_CONTENT_GAP: timeMs=", ev->getTime(), " durationMs=", ev->getDuration());
            }
            break;
        }
        case AAMP_EVENT_HTTP_RESPONSE_HEADER:
        {
            auto ev = std::dynamic_pointer_cast<HTTPResponseHeaderEvent>(e);
            eventName = "onHttpResponseHeader";
            if (ev)
            {
                params["header"] = ev->getHeader();
                params["response"] = ev->getResponse();
                LOG(LogLevel::INFO, "AAMP_EVENT_HTTP_RESPONSE_HEADER: header=", ev->getHeader());
            }
            break;
        }
        case AAMP_EVENT_CONTENT_PROTECTION_DATA_UPDATE:
        {
            auto ev = std::dynamic_pointer_cast<ContentProtectionDataEvent>(e);
            eventName = "onContentProtectionDataUpdate";
            if (ev)
            {
                params["streamType"] = ev->getStreamType();
                LOG(LogLevel::INFO, "AAMP_EVENT_CONTENT_PROTECTION_DATA_UPDATE: streamType=", ev->getStreamType());
            }
            break;
        }
        case AAMP_EVENT_MANIFEST_REFRESH_NOTIFY:
        {
            auto ev = std::dynamic_pointer_cast<ManifestRefreshEvent>(e);
            eventName = "onManifestRefresh";
            if (ev)
            {
                params["manifestDuration"] = ev->getManifestDuration();
                params["noOfPeriods"] = ev->getNoOfPeriods();
                params["manifestPublishedTime"] = ev->getManifestPublishedTime();
                params["manifestType"] = ev->getManifestType();
                LOG(LogLevel::INFO, "AAMP_EVENT_MANIFEST_REFRESH_NOTIFY: periods=", ev->getNoOfPeriods());
            }
            break;
        }
        case AAMP_EVENT_TUNE_TIME_METRICS:
        {
            auto ev = std::dynamic_pointer_cast<TuneTimeMetricsEvent>(e);
            eventName = "onTuneTimeMetrics";
            if (ev)
            {
                params["tuneMetricsData"] = ev->getTuneMetricsData();
                LOG(LogLevel::INFO, "AAMP_EVENT_TUNE_TIME_METRICS");
            }
            break;
        }
        case AAMP_EVENT_MONITORAV_STATUS:
        {
            auto ev = std::dynamic_pointer_cast<MonitorAVStatusEvent>(e);
            eventName = "onMonitorAVStatus";
            if (ev)
            {
                params["monitorAVStatus"] = ev->getMonitorAVStatus();
                params["videoPositionMs"] = ev->getVideoPositionMS();
                params["audioPositionMs"] = ev->getAudioPositionMS();
                params["timeInStateMs"] = ev->getTimeInStateMS();
                params["droppedFrames"] = ev->getDroppedFrames();
                LOG(LogLevel::INFO, "AAMP_EVENT_MONITORAV_STATUS: status=", ev->getMonitorAVStatus());
            }
            break;
        }
        default:
            // Unhandled event — no notification sent
            LOG(LogLevel::TRACE, "Unhandled AAMP event (not forwarded): ", mapAAMPEventToString(type));
            return;
        }

        LOG(LogLevel::DEBUG, "Dispatching RPC event: ", eventName);
        cb(eventName, params);
    }
} // namespace refplayer
