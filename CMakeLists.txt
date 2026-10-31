cmake_minimum_required(VERSION 3.20)

project(MossUI VERSION 1.0.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(MOSSUI_BUILD_TESTS "Build MossUI interaction tests" OFF)

file(GLOB_RECURSE MOSSUI_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/MossUI/*.cpp"
)

add_library(MossUI STATIC ${MOSSUI_SOURCES})
add_library(MossUI::MossUI ALIAS MossUI)

target_compile_features(MossUI PUBLIC cxx_std_20)
target_include_directories(MossUI
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install(TARGETS MossUI
    EXPORT MossUITargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY src/MossUI
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp"
)
install(EXPORT MossUITargets
    FILE MossUITargets.cmake
    NAMESPACE MossUI::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MossUI
)

configure_package_config_file(
    cmake/MossUIConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/MossUIConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/MossUI"
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/MossUIConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/MossUIConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/MossUIConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MossUI
)
install(FILES LICENSE DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/MossUI)

if(MOSSUI_BUILD_TESTS)
    enable_testing()
    add_executable(MossUI.Tests tests/MossUITests.cpp)
    target_link_libraries(MossUI.Tests PRIVATE MossUI::MossUI)
    add_test(NAME MossUI.Tests COMMAND MossUI.Tests)
    add_executable(MossUI.PrimitiveTests tests/PrimitiveTests.cpp)
    target_link_libraries(MossUI.PrimitiveTests PRIVATE MossUI::MossUI)
    add_test(NAME MossUI.PrimitiveTests COMMAND MossUI.PrimitiveTests)
endif()
