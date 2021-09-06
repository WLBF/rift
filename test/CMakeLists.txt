file(GLOB test_sources ${PROJECT_SOURCE_DIR}/test/*.cpp)

foreach (source_path ${test_sources})
    get_filename_component(executable_name ${source_path} NAME_WE)
    add_executable(${executable_name} ${source_path})
    target_link_libraries(${executable_name} rift GTest::GTest GTest::Main glog::glog)
    add_test(NAME ${executable_name} COMMAND ${executable_name})
endforeach (source_path ${test_sources})
