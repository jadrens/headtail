cmake_minimum_required(VERSION 3.15)
project(headtail VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(headtail main.cpp)

target_compile_options(headtail PRIVATE -Wall -Wextra)

install(TARGETS headtail DESTINATION bin)

# CPack configuration
set(CPACK_PACKAGE_NAME "headtail")
set(CPACK_PACKAGE_VENDOR "headtail")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A combined head and tail utility")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/jadrens/headtail")
set(CPACK_PACKAGE_CONTACT "jadrens")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})

# Common package settings
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

# Debian package
set(CPACK_DEBIAN_PACKAGE_NAME "headtail")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "jadrens")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "")

# RPM package
set(CPACK_RPM_PACKAGE_NAME "headtail")
set(CPACK_RPM_PACKAGE_GROUP "Utilities/System")
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_URL "https://github.com/jadrens/headtail")

# Archive (for Arch Linux pkg.tar.zst)
set(CPACK_ARCHIVE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_PROCESSOR}")

# Windows installer (WiX via CMake)
set(CPACK_WIX_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/scripts/wix-install.wxs")

include(CPack)