add_definitions(
    -D_WIN32_WINNT=0x0602
    -DWINVER=0x0602
    -DNOMINMAX
    -D_SILENCE_CXX17_C_HEADER_DEPRECATION_WARNING
    # disable: warning C4275: non dll-interface class '***' used as base for dll-interface clas '***'
    -D_SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING
    # If deprecated API is enabled compilation will fail with VS2017
    /wd4275
    /wd4996
)

# Set Windows target platform version
# For an overview of platform versions:
# https://en.wikipedia.org/wiki/Microsoft_Windows_SDK#Features
set (CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION "10.0.17134.0" CACHE STRING "")

# Issue with VC and disabling of C4200: https://connect.microsoft.com/VisualStudio/feedback/details/1114440
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4200")

# Make VC set _cplusplus value standard conformant: /Zc:__cplusplus
# Article: https://devblogs.microsoft.com/cppblog/msvc-now-correctly-reports-__cplusplus/
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus")

# Until the issue with json library is fixed, ignore deprecation warnings
# See: https://github.com/nlohmann/json/issues/1695
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996")
