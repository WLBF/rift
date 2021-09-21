file(GLOB example_sources ${PROJECT_SOURCE_DIR}/example/*.cpp)

foreach (source_path ${example_sources})
    get_filename_component(executable_name ${source_path} NAME_WE)
    add_executable(${executable_name} ${source_path})
    target_link_libraries(${executable_name} rift glog::glog)
endforeach (source_path ${example_sources})
