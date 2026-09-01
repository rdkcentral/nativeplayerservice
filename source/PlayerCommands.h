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

#ifndef REFPLAYER_PLAYERCOMMANDS_H
#define REFPLAYER_PLAYERCOMMANDS_H
#include <string>
// This class is intended to hold the method names that are available for the NRPWS RPC server.
// It is a static class, so no instances of it should be created.
//NRP is for native reference player

#define NRP_URN_BASE "org.rdk.player"
#define NRP_METHOD_BASE "org.rdk.player."
namespace refplayer
{

    class PlayerCommands
    {

    public:
        static constexpr auto NRP_METHOD_REGISTER = NRP_METHOD_BASE "register";
        static constexpr auto NRP_METHOD_UNREGISTER = NRP_METHOD_BASE "unregister";
        static constexpr auto NRP_METHOD_GET_LISTENERS = NRP_METHOD_BASE "getListeners";

        // Player related methods
        static constexpr auto NRP_METHOD_OPEN_SESSION = NRP_METHOD_BASE "openSession";
        static constexpr auto NRP_METHOD_GET_SESSION_INFO = NRP_METHOD_BASE "getSessionInfo";
        static constexpr auto NRP_METHOD_SETUP_SESSION = NRP_METHOD_BASE "setupSession";
        static constexpr auto NRP_METHOD_PLAY = NRP_METHOD_BASE "play";
        static constexpr auto NRP_METHOD_STOP = NRP_METHOD_BASE "stop";
        static constexpr auto NRP_METHOD_SEEK = NRP_METHOD_BASE "seek";
        static constexpr auto NRP_METHOD_SEEK_TO_LIVE = NRP_METHOD_BASE "seekToLive";
        static constexpr auto NRP_METHOD_SET_RATE = NRP_METHOD_BASE "setRate";
        static constexpr auto NRP_METHOD_SET_PLAYBACK_SPEED = NRP_METHOD_BASE "setPlaybackSpeed";
        static constexpr auto NRP_METHOD_PAUSE_AT = NRP_METHOD_BASE "pauseAt";
        static constexpr auto NRP_METHOD_SET_RATE_AND_SEEK = NRP_METHOD_BASE "setRateAndSeek";
        static constexpr auto NRP_METHOD_GET_STATE = NRP_METHOD_BASE "getState";
        static constexpr auto NRP_METHOD_GET_PLAYBACK_POSITION = NRP_METHOD_BASE "getPlaybackPosition";
        static constexpr auto NRP_METHOD_GET_PLAYBACK_DURATION = NRP_METHOD_BASE "getPlaybackDuration";
        static constexpr auto NRP_METHOD_GET_PLAYBACK_RATE = NRP_METHOD_BASE "getPlaybackRate";
        static constexpr auto NRP_METHOD_CLOSE_SESSION = NRP_METHOD_BASE "closeSession";

        // Playback State
        static constexpr auto NRP_METHOD_IS_LIVE = NRP_METHOD_BASE "isLive";

        // Video
        static constexpr auto NRP_METHOD_SET_VIDEO_MUTE = NRP_METHOD_BASE "setVideoMute";
        static constexpr auto NRP_METHOD_GET_VIDEO_MUTE = NRP_METHOD_BASE "getVideoMute";

        // Audio
        static constexpr auto NRP_METHOD_SET_AUDIO_VOLUME = NRP_METHOD_BASE "setAudioVolume";
        static constexpr auto NRP_METHOD_GET_AUDIO_VOLUME = NRP_METHOD_BASE "getAudioVolume";
        static constexpr auto NRP_METHOD_GET_AUDIO_LANGUAGE = NRP_METHOD_BASE "getAudioLanguage";
        static constexpr auto NRP_METHOD_GET_AVAILABLE_AUDIO_TRACKS = NRP_METHOD_BASE "getAvailableAudioTracks";
        static constexpr auto NRP_METHOD_SET_AUDIO_TRACK = NRP_METHOD_BASE "setAudioTrack";
        static constexpr auto NRP_METHOD_GET_AUDIO_TRACK = NRP_METHOD_BASE "getAudioTrack";
        static constexpr auto NRP_METHOD_GET_AUDIO_TRACK_INFO = NRP_METHOD_BASE "getAudioTrackInfo";

        // Subtitles
        static constexpr auto NRP_METHOD_SET_SUBTITLE_MUTE = NRP_METHOD_BASE "setSubtitleMute";
        static constexpr auto NRP_METHOD_GET_AVAILABLE_TEXT_TRACKS = NRP_METHOD_BASE "getAvailableTextTracks";
        static constexpr auto NRP_METHOD_SET_TEXT_TRACK = NRP_METHOD_BASE "setTextTrack";
        static constexpr auto NRP_METHOD_GET_TEXT_TRACK = NRP_METHOD_BASE "getTextTrack";

        // Bitrate / ABR
        static constexpr auto NRP_METHOD_GET_VIDEO_BITRATE = NRP_METHOD_BASE "getVideoBitrate";
        static constexpr auto NRP_METHOD_SET_VIDEO_BITRATE = NRP_METHOD_BASE "setVideoBitrate";
        static constexpr auto NRP_METHOD_GET_VIDEO_BITRATES = NRP_METHOD_BASE "getVideoBitrates";
        static constexpr auto NRP_METHOD_SET_INITIAL_BITRATE = NRP_METHOD_BASE "setInitialBitrate";
        static constexpr auto NRP_METHOD_GET_INITIAL_BITRATE = NRP_METHOD_BASE "getInitialBitrate";
        static constexpr auto NRP_METHOD_SET_MINIMUM_BITRATE = NRP_METHOD_BASE "setMinimumBitrate";
        static constexpr auto NRP_METHOD_GET_MINIMUM_BITRATE = NRP_METHOD_BASE "getMinimumBitrate";
        static constexpr auto NRP_METHOD_SET_MAXIMUM_BITRATE = NRP_METHOD_BASE "setMaximumBitrate";
        static constexpr auto NRP_METHOD_GET_MAXIMUM_BITRATE = NRP_METHOD_BASE "getMaximumBitrate";

        // DRM
        static constexpr auto NRP_METHOD_SET_LICENSE_SERVER_URL = NRP_METHOD_BASE "setLicenseServerURL";
        static constexpr auto NRP_METHOD_GET_DRM = NRP_METHOD_BASE "getDRM";
        static constexpr auto NRP_METHOD_SET_PREFERRED_DRM = NRP_METHOD_BASE "setPreferredDRM";

        // Configuration
        static constexpr auto NRP_METHOD_CONFIGURE_SESSION = NRP_METHOD_BASE "configureSession";
        static constexpr auto NRP_METHOD_GET_AAMP_CONFIG = NRP_METHOD_BASE "getAAMPConfig";
        static constexpr auto NRP_METHOD_SET_APP_NAME = NRP_METHOD_BASE "setAppName";
        static constexpr auto NRP_METHOD_SET_PREFERRED_LANGUAGES = NRP_METHOD_BASE "setPreferredLanguages";
        static constexpr auto NRP_METHOD_GET_PREFERRED_LANGUAGES = NRP_METHOD_BASE "getPreferredLanguages";
    };

} // namespace refplayer

#endif // REFPLAYER_PLAYERCOMMANDS_H