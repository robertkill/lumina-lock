# Lumina Lock control-center plugin (V25 QML plugin architecture).
# Built only when the dde-control-center dev package is available.
cmake_minimum_required(VERSION 3.23)

set(PLUGIN_NAME "luminalock")

find_package(Qt6 COMPONENTS Core REQUIRED)
find_package(DdeControlCenter REQUIRED)
find_package(Dtk6Core REQUIRED)

file(GLOB_RECURSE PLUGIN_SRCS
    "src/*.cpp"
    "src/*.h"
)

add_library(${PLUGIN_NAME} MODULE ${PLUGIN_SRCS})

# 模块图标作为资源注册成 :/dsg/icons/luminalock.dci
set(CMAKE_AUTORCC ON)
target_sources(${PLUGIN_NAME} PRIVATE dcc-icons.qrc)

target_link_libraries(${PLUGIN_NAME} PRIVATE
    Dde::Control-Center
    Qt6::Core
    Dtk6::Core
)

# Builds the QML module (lib${PLUGIN_NAME}_qml.so) and installs the plugin
# into plugins_v1.1/${PLUGIN_NAME}/ alongside the .so and qmldir.
dcc_install_plugin(NAME ${PLUGIN_NAME} TARGET ${PLUGIN_NAME})
