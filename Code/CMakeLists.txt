project(Template)

add_executable(${PROJECT_NAME})

target_compile_definitions(${PROJECT_NAME} PRIVATE USE_OPENGL)
target_link_libraries(${PROJECT_NAME}      PRIVATE Core Platform Graphics)
target_sources(${PROJECT_NAME}             PRIVATE main.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Build")