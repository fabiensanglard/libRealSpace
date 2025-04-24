file (GLOB SC_SOURCE "*.cpp")
file (GLOB SC_HEADER "*.h")

add_executable (debugger ${SC_SOURCE} ${SC_HEADER})

target_include_directories(debugger PUBLIC ${SC_INCLUDES})
target_include_directories(debugger PUBLIC ${SDL2_INCLUDE_DIRS})
target_include_directories(debugger PUBLIC ${OPENGL_INCLUDE_DIR})
target_include_directories(debugger PUBLIC ${OPENGL_INCLUDE_DIR})
target_link_libraries(debugger PRIVATE engine realspace commons)
target_link_libraries(debugger PUBLIC ${SDL2_LIBRARIES})
target_link_libraries(debugger PRIVATE imgui::imgui )
target_link_libraries(debugger PUBLIC ${OPENGL_LIBRARIES})
