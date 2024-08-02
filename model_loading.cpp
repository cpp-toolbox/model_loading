
#include <spdlog/spdlog.h>
#include <assimp/postprocess.h>

#include "model_loading.hpp"
#include "sbpt_generated_includes.hpp"

Model ModelLoader::load_model(const std::string &path) {

    std::function<void(aiMesh *, const aiScene*)> process_step = [this](aiMesh *mesh, const aiScene *scene) {
        spdlog::get(Systems::asset_loading)->info("process step working");
        this->meshes.push_back(this->process_mesh(mesh, scene));
    };

    std::function<void(aiNode *, const aiScene *)> recursively_process_nodes = ModelLoader::recursively_process_nodes_closure(process_step);

    this->call_function_with_assimp_importer_context(path, [&](auto root, auto scene) {
        spdlog::get(Systems::asset_loading)->info("starting to process nodes");
        recursively_process_nodes(root, scene);
        spdlog::get(Systems::asset_loading)->info("processed all nodes");
    });

    return Model(this->meshes);
};

/**
 * /note this function is guaranteed to terminate because all models are finite and non-cyclic
 */
std::function<void(aiNode *, const aiScene *)>
ModelLoader::recursively_process_nodes_closure(const std::function<void(aiMesh *, const aiScene *)>& process_function) {
    std::function<void(aiNode * node, const aiScene *scene)> recursively_process_nodes = [&](aiNode *node,
                                                                                             const aiScene *scene) {
        spdlog::get(Systems::asset_loading)->info("started processing meshes");
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            unsigned int mesh_index = node->mMeshes[i];
            aiMesh *mesh = scene->mMeshes[mesh_index];
            process_function(mesh, scene);
        }
        spdlog::get(Systems::asset_loading)->info("finished processing meshes");
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            recursively_process_nodes(node->mChildren[i], scene);
        }
    };
    return recursively_process_nodes;
};

Mesh ModelLoader::process_mesh(aiMesh *mesh, const aiScene *scene) {
    std::vector<glm::vec3> vertices = this->process_mesh_vertex_positions(mesh);
    std::vector<unsigned int> indices = this->process_mesh_indices(mesh);
    return {vertices, indices};
};

glm::vec3 assimp_to_glm_3d_vector(aiVector3D assimp_vector) {
    return {assimp_vector.x, assimp_vector.y, assimp_vector.z};
}

std::vector<glm::vec3> ModelLoader::process_mesh_vertex_positions(aiMesh *mesh) {
    std::vector<glm::vec3> vertices;
    spdlog::get(Systems::asset_loading)->info("This mesh has {} vertex_positions", mesh->mNumVertices);
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        glm::vec3 vertex = {assimp_to_glm_3d_vector(mesh->mVertices[i])};
        vertices.push_back(vertex);
    }
    return vertices;
}

std::vector<unsigned int> ModelLoader::process_mesh_indices(aiMesh *mesh) {
    std::vector<unsigned int> indices;
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        assert(face.mNumIndices == 3); // if this is false we are not working with triangles
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }
    return indices;
}
