file(GLOB LIBRARY_SOURCES "*.cpp")
file(GLOB LIBRARY_HEADERS "*.h")


add_library(strike_commander STATIC ${LIBRARY_SOURCES} ${LIBRARY_HEADERS})

target_include_directories(strike_commander PUBLIC ${SDL2_INCLUDE_DIRS})
target_include_directories(strike_commander PUBLIC ${OPENGL_INCLUDE_DIR})
target_link_libraries(strike_commander PRIVATE $<IF:$<TARGET_EXISTS:SDL2_mixer_ext::SDL2_mixer_ext>,SDL2_mixer_ext::SDL2_mixer_ext,SDL2_mixer_ext::SDL2_mixer_ext_Static>)
target_include_directories(strike_commander PUBLIC ${LIBRARY_INCLUDES})
target_link_libraries(strike_commander PUBLIC ${LIBRARY_LINKS})