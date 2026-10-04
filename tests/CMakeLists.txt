option(ENABLE__TEST "Enable Unit Tests")

if (ENABLE_TEST)
    message("Test suite is enabled")
    add_subdirectory(demo)
endif ()