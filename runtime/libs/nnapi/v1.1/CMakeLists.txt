add_library(nnfw_lib_nnapi_1_1 INTERFACE)

target_include_directories(nnfw_lib_nnapi_1_1 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(nnfw_lib_nnapi_1_1 INTERFACE nnfw-nnapi-header)
