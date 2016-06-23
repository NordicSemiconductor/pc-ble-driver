add_definitions(
    -D_WIN32_WINNT=0x0502
    -DNOMINMAX
)

# Issue with VC and disabling of C4200: https://connect.microsoft.com/VisualStudio/feedback/details/1114440
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4200")

