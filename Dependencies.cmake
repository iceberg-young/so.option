add_subdirectory(import/json/source)
add_dependencies(${PROJECT_NAME} so.json)
target_link_libraries(${PROJECT_NAME} so.json)
include_directories(import/json/include)

add_subdirectory(import/json/import/utf/source)
add_dependencies(${PROJECT_NAME} so.utf)
target_link_libraries(${PROJECT_NAME} so.utf)
