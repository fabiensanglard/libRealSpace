file (GLOB SC_SOURCE "*.cpp")
file (GLOB SC_HEADER "*.h")

add_executable ("sc" PUBLIC ${SC_SOURCE} ${SC_HEADER})

target_include_directories("sc" PUBLIC ${SC_INCLUDES})
target_include_directories("LibRealspace" PUBLIC ${LIBRARY_INCLUDES})
target_link_libraries("sc" PRIVATE "LibRealspace")