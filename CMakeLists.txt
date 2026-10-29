cmake_minimum_required(VERSION 3.21)
project(QuirewiseCharset VERSION 0.1.0 LANGUAGES C)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CTest)

add_library(quirewise_charset STATIC src/charset.c)
add_library(Quirewise::Charset ALIAS quirewise_charset)
set_target_properties(quirewise_charset PROPERTIES
    EXPORT_NAME Charset
    POSITION_INDEPENDENT_CODE ON)
target_compile_features(quirewise_charset PUBLIC c_std_11)
target_include_directories(quirewise_charset PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

if(BUILD_TESTING)
    add_executable(test_charset tests/test_charset.c)
    target_compile_features(test_charset PRIVATE c_std_11)
    target_link_libraries(test_charset PRIVATE Quirewise::Charset)
    add_test(NAME charset_vectors COMMAND test_charset)
endif()

install(TARGETS quirewise_charset EXPORT QuirewiseCharsetTargets
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(DIRECTORY include/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(EXPORT QuirewiseCharsetTargets
    FILE QuirewiseCharsetTargets.cmake
    NAMESPACE Quirewise::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/QuirewiseCharset")

configure_package_config_file(
    cmake/QuirewiseCharsetConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/QuirewiseCharsetConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/QuirewiseCharset")
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/QuirewiseCharsetConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/QuirewiseCharsetConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/QuirewiseCharsetConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/QuirewiseCharset")
