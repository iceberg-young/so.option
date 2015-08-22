add_subdirectory(import/setting/source)
add_dependencies(${PROJECT_NAME} so.setting)
target_link_libraries(${PROJECT_NAME} so.setting)
include_directories(import/setting/include)

add_subdirectory(import/json/source)
add_dependencies(${PROJECT_NAME} so.json)
target_link_libraries(${PROJECT_NAME} so.json)
include_directories(import/json/include)

add_subdirectory(import/json/import/utf/source)
add_dependencies(${PROJECT_NAME} so.utf)
target_link_libraries(${PROJECT_NAME} so.utf)
