file (GLOB SC_SOURCE "*.cpp")
file (GLOB SC_HEADER "*.h")

add_executable (neosc ${SC_SOURCE} ${SC_HEADER})

target_include_directories(neosc PUBLIC ${SC_INCLUDES})
target_include_directories(neosc PUBLIC ${SDL2_INCLUDE_DIRS})
target_include_directories(neosc PUBLIC ${OPENGL_INCLUDE_DIR})
target_include_directories(neosc PUBLIC ${OPENGL_INCLUDE_DIR})
target_link_libraries(neosc PRIVATE strike_commander engine realspace commons)
target_link_libraries(neosc PUBLIC ${SDL2_LIBRARIES})
target_link_libraries(neosc PRIVATE imgui::imgui )
target_link_libraries(neosc PUBLIC ${OPENGL_LIBRARIES})
