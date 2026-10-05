include_guard()

find_program(CLANG_FORMATTER NAMES clang-format)
if (CLANG_FORMATTER)
    file(GLOB_RECURSE ALL_SOURCE_FILES *.cpp *.h *.cxx *.hxx *.hpp *.cc *.hh *.tpp *.inl)

    add_custom_target(${PROJECT_NAME}-Format
            COMMAND ${CLANG_FORMATTER} -i -style=file ${ALL_SOURCE_FILES}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Formatting source files..."
    )
    set_target_properties(${PROJECT_NAME}-Format PROPERTIES FOLDER ${DevToolsDir})
else ()
    message(WARNING "clang-format not found")
endif ()