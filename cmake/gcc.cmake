add_compile_options(
    -Wall
    -Wno-effc++
    -Wno-unknown-pragmas
    -Wno-undef
    -Wno-long-long
    -Wfloat-equal
    -Wpointer-arith
    #-H # Used for debugging header dependencies. See https://docs.freebsd.org/info/gcc/gcc.info.Preprocessor_Options.html
)

add_compile_options(
    -std=c++11
    -Wlogical-op
)
