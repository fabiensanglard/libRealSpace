file(GLOB LIBRARY_SOURCES "*.cpp")
file(GLOB LIBRARY_HEADERS "*.h")


add_library(librealspace STATIC ${LIBRARY_SOURCES} ${LIBRARY_HEADERS})


target_include_directories(librealspace PUBLIC ${SDL2_INCLUDE_DIRS})
target_include_directories(librealspace PUBLIC ${OPENGL_INCLUDE_DIR})
target_include_directories(librealspace PUBLIC ${LIBRARY_INCLUDES})
target_link_libraries(librealspace PUBLIC ${LIBRARY_LINKS})