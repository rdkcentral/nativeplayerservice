# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2026 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Aamp REQUIRED)
find_package( jsoncpp REQUIRED)

find_package(FireboltTransport CONFIG REQUIRED)
find_package(FireboltClient CONFIG REQUIRED)

find_package(wsrpcserver REQUIRED)


pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0>=1.18.0)
pkg_check_modules(GSTREAMERBASE REQUIRED gstreamer-app-1.0)
pkg_check_modules(GSTREAMERVIDEO REQUIRED gstreamer-video-1.0)

include_directories(${GLIB_INCLUDE_DIRS} ${GSTREAMER_INCLUDE_DIRS} ${GSTREAMERBASE_INCLUDE_DIRS} ${GSTREAMERVIDEO_INCLUDE_DIRS})

set(NATIVEPLAYER_SOURCES)
list(APPEND NATIVEPLAYER_SOURCES
    Application.cpp
    main.cpp
    Player.cpp
    FireboltConnector.cpp
    PlayerDelegate.cpp
)

list(APPEND NATIVEPLAYER_SOURCES
    WebSocketConnector.cpp
)
add_executable(nativeplayer ${NATIVEPLAYER_SOURCES})

target_include_directories(nativeplayer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(nativeplayer
    PRIVATE
        ${GLIB2_LIBRARIES}
        ${GSTREAMER_LIBRARIES}
        ${GSTREAMER_BASE_LIBRARIES}
        ${GSTREAMER_VIDEO_LIBRARIES}
        ${GSTREAMER_APP_LIBRARIES}
        AAMP::AAMP        
        JsonCpp::JsonCpp
        uuid
        FireboltTransport::FireboltTransport
        FireboltClient::FireboltClient
        WSRPCSERVER::WSRPCSERVER
)

install(TARGETS nativeplayer
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
if(BUILD_TESTING)
    # nativeplayer runs until SIGTERM/SIGINT, so avoid running it as a CTest.
endif()
