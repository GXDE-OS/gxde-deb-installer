cmake_minimum_required(VERSION 3.7)

if (NOT DEFINED VERSION)
    set(VERSION 1.2.2)
endif ()

project(gxde-deb-installer)

#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

if (NOT (${CMAKE_BUILD_TYPE} MATCHES "Debug"))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

    # generate qm files
    execute_process(COMMAND bash "translate_generation.sh"
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

    execute_process(COMMAND bash "translate_ts2desktop.sh"
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
else ()
    set(CMAKE_CXX_COMPILER "clang++")
endif ()

configure_file(environments.h.in environments.h @ONLY)

# Sources files
file(GLOB SRCS "*.h" "*.cpp" "widgets/*.h" "widgets/*.cpp")

# Find the library
find_package(PkgConfig REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Widgets Concurrent)
pkg_check_modules(QApt REQUIRED IMPORTED_TARGET libqapt-qt6)
find_package(Dtk2widget REQUIRED)

# Tell CMake to create the executable
add_executable(gxde-deb-installer ${SRCS} resources/resources.qrc)
target_include_directories(gxde-deb-installer PUBLIC ${Dtk2widget_INCLUDE_DIRS} ${Dtk6Core_INCLUDE_DIRS} ${OBJECT_BINARY_DIR})
target_link_libraries(gxde-deb-installer
    PkgConfig::QApt
    Dtk6::Core
    ${Dtk2widget_LIBRARIES}
    Qt6::Widgets
    Qt6::Concurrent
)

# Install settings
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

# Install qm files
file(GLOB QM_FILES "translations/*.qm")
install(FILES ${QM_FILES}
        DESTINATION share/gxde-deb-installer/translations)

# Install icons
foreach (ICON_SIZE 16 24 32 48 64 96 128 256)
    install(FILES resources/images/gxde-deb-installer-${ICON_SIZE}px.svg
            RENAME gxde-deb-installer.svg
            DESTINATION share/icons/hicolor/${ICON_SIZE}x${ICON_SIZE}/apps)
endforeach ()

install(TARGETS gxde-deb-installer DESTINATION bin)
install(FILES gxde-deb-installer.desktop DESTINATION share/applications)
install(FILES gxde-deb-installer.applications DESTINATION share/application-registry)
