file (GLOB SC_SOURCE "*.cpp")
file (GLOB SC_HEADER "*.h")

add_executable (neoto ${SC_SOURCE} ${SC_HEADER})

target_include_directories(neoto PUBLIC ${SC_INCLUDES})
target_include_directories(neoto PUBLIC ${SDL2_INCLUDE_DIRS})
target_include_directories(neoto PUBLIC ${OPENGL_INCLUDE_DIR})
target_include_directories(neoto PUBLIC ${OPENGL_INCLUDE_DIR})
target_link_libraries(neoto PRIVATE engine realspace commons)
target_link_libraries(neoto PUBLIC ${SDL2_LIBRARIES})
target_link_libraries(neoto PRIVATE imgui::imgui )
target_link_libraries(neoto PUBLIC ${OPENGL_LIBRARIES})
