project(Template)

add_executable(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE Platform glm)
target_sources(${PROJECT_NAME}        PRIVATE main.cpp)