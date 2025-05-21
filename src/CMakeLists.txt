add_library(vt
    pxr/vt/array.cpp
    pxr/vt/dictionary.cpp
    pxr/vt/hash.cpp
    pxr/vt/streamOut.cpp
    pxr/vt/types.cpp
    pxr/vt/value.cpp
)

target_include_directories(vt
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(vt
    PUBLIC
        pxr::arch
        pxr::tf
        pxr::gf
        pxr::trace
        TBB::tbb
)

if(BUILD_PYTHON_BINDINGS)
    target_sources(vt
        PRIVATE
            pxr/vt/moduleDeps.cpp
            pxr/vt/arrayPyBuffer.cpp
            pxr/vt/valueFromPython.cpp
            pxr/vt/wrapArray.cpp
    )

    target_link_libraries(vt
        PUBLIC
            pxr::boost-python
    )

    add_subdirectory(python)
endif()

if (ENABLE_PRECOMPILED_HEADERS)
    target_precompile_headers(vt
        PRIVATE
            pxr/vt/pch.h
    )
endif()

set_target_properties(vt
    PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        OUTPUT_NAME "PxrVt"
)

target_compile_definitions(vt
    PRIVATE
        VT_EXPORTS=1
        MFB_PACKAGE_MODULE=Vt
        MFB_PACKAGE_NAME=vt
        MFB_ALT_PACKAGE_NAME=vt
)

install(
    TARGETS vt EXPORT ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
    FILES
        pxr/vt/api.h
        pxr/vt/array.h
        pxr/vt/dictionary.h
        pxr/vt/hash.h
        pxr/vt/streamOut.h
        pxr/vt/traits.h
        pxr/vt/typeHeaders.h
        pxr/vt/types.h
        pxr/vt/value.h
        pxr/vt/visitValue.h
    DESTINATION
        ${CMAKE_INSTALL_INCLUDEDIR}/pxr/vt
)

if(BUILD_PYTHON_BINDINGS)
    install(
        FILES
            pxr/vt/pyOperators.h
        DESTINATION
            ${CMAKE_INSTALL_INCLUDEDIR}/pxr/vt
    )
endif()

install(EXPORT ${PROJECT_NAME}
    FILE pxr-vt-targets.cmake
    NAMESPACE pxr::
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/cmake/pxr-vt
)
