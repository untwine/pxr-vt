if (WIN32)
    # Patch CMake's 'add_test' to extend the DLL search path on Windows. Use the
    # target name associated with the test, if it is also a CMake target.

    macro(add_test)
        _add_test(${ARGV})

        cmake_parse_arguments("" "" "NAME" "" ${ARGV})
        if (TARGET ${_NAME})
            set(DLL_DIRS $<TARGET_RUNTIME_DLL_DIRS:${_NAME}>)
            set_tests_properties(${_NAME}
                PROPERTIES ENVIRONMENT
                "PATH=$<JOIN:$<SHELL_PATH:${DLL_DIRS}>,\\;>")
        endif()
    endmacro()
endif()

add_executable(testVtCpp testVtCpp.cpp)
target_link_libraries(testVtCpp PUBLIC vt)
add_test(NAME testVtCpp COMMAND testVtCpp)

add_executable(testVtArrayEditCpp testVtArrayEdit.cpp)
target_link_libraries(testVtArrayEditCpp PUBLIC vt)
add_test(NAME testVtArrayEditCpp COMMAND testVtArrayEditCpp)

if(BUILD_PYTHON_BINDINGS)
    pytest_discover_tests(
        testPyVt
        LIBRARY_PATH_PREPEND
            $<TARGET_FILE_DIR:vt>
            $<TARGET_FILE_DIR:pyVt>
            $<TARGET_FILE_DIR:pxr::gf>
            $<TARGET_FILE_DIR:pxr::pyGf>
            $<TARGET_FILE_DIR:pxr::tf>
            $<TARGET_FILE_DIR:pxr::pyTf>
        PYTHON_PATH_PREPEND
            "$<TARGET_FILE_DIR:pyVt>/../.."
            "$<TARGET_FILE_DIR:pxr::pyGf>/../.."
            "$<TARGET_FILE_DIR:pxr::pyTf>/../.."
        TRIM_FROM_NAME "^(TestVt|test_)"
        EXTRA_ARGS "-v"
        DEPENDS vt pyVt
    )
endif()