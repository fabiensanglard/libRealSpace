file (GLOB SC_SOURCE "*.cpp")
file (GLOB SC_HEADER "*.h")

add_executable (neosc ${SC_SOURCE} ${SC_HEADER})

if(MSVC)
    target_link_options(neosc PUBLIC /DEBUG)
endif()

target_include_directories(neosc PUBLIC ${SC_INCLUDES})
target_include_directories(neosc PUBLIC ${SDL2_INCLUDE_DIRS})
target_include_directories(neosc PUBLIC ${OPENGL_INCLUDE_DIR})
target_link_libraries(neosc PRIVATE librealspace)
target_link_libraries(neosc PUBLIC ${SDL2_LIBRARIES})
target_link_libraries(neosc PUBLIC ${OPENGL_LIBRARIES})
