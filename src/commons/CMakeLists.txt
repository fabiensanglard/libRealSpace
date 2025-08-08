file(GLOB LIBRARY_SOURCES "*.cpp")
file(GLOB LIBRARY_HEADERS "*.h")


add_library(commons STATIC ${LIBRARY_SOURCES} ${LIBRARY_HEADERS})

target_include_directories(commons PUBLIC ${SDL2_INCLUDE_DIRS})
target_include_directories(commons PUBLIC ${OPENGL_INCLUDE_DIR})
target_link_libraries(commons PRIVATE $<IF:$<TARGET_EXISTS:SDL2_mixer_ext::SDL2_mixer_ext>,SDL2_mixer_ext::SDL2_mixer_ext,SDL2_mixer_ext::SDL2_mixer_ext_Static>)
target_include_directories(commons PUBLIC ${LIBRARY_INCLUDES})
target_link_libraries(commons PUBLIC ${LIBRARY_LINKS})