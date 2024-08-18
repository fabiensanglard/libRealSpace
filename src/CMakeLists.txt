file(GLOB LIBRARY_SOURCES "*.cpp")
file(GLOB LIBRARY_HEADERS "*.h")


add_library("LibRealSpace" STATIC ${LIBRARY_SOURCES} ${LIBRARY_HEADERS})

target_include_directories("LibRealSpace" PUBLIC ${LIBRARY_INCLUDES})
target_link_libraries("LibRealSpace" PUBLIC ${LIBRARY_LINKS})