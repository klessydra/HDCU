set(SOURCES
    src/hdc_class.cpp
    src/hv_struct.cpp
    )

set(HEADERS
    inc/hdc_class.hpp
    inc/hv_struct.hpp
    inc/hdc_tests.hpp
    inc/hdc_defines.hpp
    )

include_directories(inc/)

add_library(hdc_libs STATIC ${SOURCES} ${HEADERS})

