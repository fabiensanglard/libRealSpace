file(GLOB LIBRARY_SOURCES "*.cpp")
file(GLOB LIBRARY_HEADERS "*.h")


add_library(librealspace STATIC ${LIBRARY_SOURCES} ${LIBRARY_HEADERS})

target_include_directories(librealspace PUBLIC ${SDL2_INCLUDE_DIRS})
target_include_directories(librealspace PUBLIC ${OPENGL_INCLUDE_DIR})
target_link_libraries(librealspace PRIVATE $<IF:$<TARGET_EXISTS:SDL2_mixer_ext::SDL2_mixer_ext>,SDL2_mixer_ext::SDL2_mixer_ext,SDL2_mixer_ext::SDL2_mixer_ext_Static>)
target_include_directories(librealspace PUBLIC ${LIBRARY_INCLUDES})
target_link_libraries(librealspace PUBLIC ${LIBRARY_LINKS})