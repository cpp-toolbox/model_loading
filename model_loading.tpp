#include <iostream>
#include <utility>
#include <spdlog/spdlog.h>

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"

#include "model_loading.hpp"
#include "sbpt_generated_includes.hpp"

template <typename func>
void ModelLoader::call_function_with_assimp_importer_context(const std::string &path, func fn) {
    spdlog::get(Systems::asset_loading)->info("starting to process: {}", path);
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        spdlog::get(Systems::asset_loading)->error("assimp error: {}", importer.GetErrorString());
        throw std::runtime_error("assimp error");
    }
    this->directory = path.substr(0, path.find_last_of("/"));
    fn(scene->mRootNode, scene);
}
