cmake_minimum_required(VERSION 3.10)
project(StockAnalyzer)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# 设置vcpkg工具链文件路径相关的公共部分作为变量
set(VCPKG_INSTALL_DIR "C:/Users/Sharky/Desktop/vcpkg/installed/x64-windows-static")

# 设置 vcpkg 工具链
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_INSTALL_DIR}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

# 设置各个库的相关路径
set(wxWidgets_DIR "${VCPKG_INSTALL_DIR}/share/wxwidgets")
set(NanoSVG_DIR "${VCPKG_INSTALL_DIR}/share/nanosvg")
set(CURL_DIR "${VCPKG_INSTALL_DIR}/share/curl")

# 设置zlib相关路径
set(ZLIB_INCLUDE_DIR "${VCPKG_INSTALL_DIR}/include")
set(ZLIB_LIBRARY_RELEASE "${VCPKG_INSTALL_DIR}/lib/zlib.lib")

# 强制链接静态库
set(BUILD_SHARED_LIBS OFF)
set(wxWidgets_USE_STATIC ON)

# 找到 wxWidgets 和 CURL 静态库
find_package(wxWidgets CONFIG REQUIRED COMPONENTS core base)
find_package(CURL CONFIG REQUIRED)

# 包含头文件目录
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party
    ${CURL_INCLUDE_DIRS}
)

# 添加源文件
file(GLOB SOURCES "src/*.cpp")

# 创建可执行文件
add_executable(StockAnalyzer ${SOURCES})

# 链接静态库
target_link_libraries(StockAnalyzer PRIVATE
    wx::core
    wx::base
    ${CURL_LIBRARIES}
)

# 设置预处理宏
add_definitions(-D__WXMSW__)

if (MSVC)
    # 强制 MSVC 使用静态运行时库
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

    # 设置 Windows GUI 子系统
    set_target_properties(StockAnalyzer PROPERTIES
        WIN32_EXECUTABLE TRUE
        LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
    )

    # 添加编译器警告
    target_compile_options(StockAnalyzer PRIVATE /W4)

    # 强制所有配置使用静态运行时库
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")

    # 设置图标
    target_sources(StockAnalyzer PRIVATE resources.rc)

else()

    # 设置 Windows GUI 子系统
    target_link_options(StockAnalyzer PRIVATE -mwindows)

    # 添加编译器警告
    target_compile_options(StockAnalyzer PRIVATE -Wall -Wextra)
endif()