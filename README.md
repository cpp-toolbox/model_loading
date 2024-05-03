# Info

Loads in OBJ files only using vertex data

# Dependencies

* [assimp](https://github.com/assimp/assimp)
* [glm](https://github.com/g-truc/glm)

# CMake

```
...

# ASSIMP: Asset Importer
# only use obj to speed up compilation
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)
include_directories(external_libraries/assimp/include)
add_subdirectory(external_libraries/assimp)

# GLM: opengl mathematics
include_directories(external_libraries/glm)
add_subdirectory(external_libraries/glm)

... 

target_link_libraries(your_project_name ... assimp)
```
