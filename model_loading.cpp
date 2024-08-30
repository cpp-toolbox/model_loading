#include <spdlog/spdlog.h>
#include <assimp/postprocess.h>

#include "model_loading.hpp"

/**
 * @brief returns vertex positions in the order specified by the indices
 *
 * @return said vertex positions
 */
std::vector<glm::vec3> get_ordered_vertex_positions(std::vector<glm::vec3> &vertex_positions,
                                                    std::vector<unsigned int> &indices) {
    std::vector<glm::vec3> ovp;
    // TODO we don't need a nested loop here I believe...
    for (size_t i = 0; i < indices.size(); i += 3) {
        for (size_t j = 0; j < 3; j++) {
            ovp.push_back(vertex_positions[indices[i + j]]);
        }
    }
    return ovp;
}

/**
 * @brief returns a list of lists of the form specified by \ref get_ordered_vertex_positions
 *
 * @return as mentioned in brief
 */
std::vector<std::vector<glm::vec3>> Model::get_ordered_vertex_positions_for_each_mesh() {
    std::vector<std::vector<glm::vec3>> ovpfem;
    for (auto &mesh : meshes) {
        ovpfem.push_back(get_ordered_vertex_positions(mesh.vertex_positions, mesh.indices));
    }
    return ovpfem;
}

Model ModelLoader::load_model(const std::string &path) {
    auto process_step = [this](aiMesh *mesh, const aiScene *scene) {
        this->meshes.push_back(this->process_mesh(mesh, scene));
    };

    auto recursively_process_nodes = this->recursively_process_nodes_closure(process_step);

    if (!recursively_process_nodes) {
        // Handle the error or throw a custom exception
        throw std::runtime_error("Attempted to call an uninitialized function");
    }

    this->call_function_with_assimp_importer_context(path, [&](auto root, auto scene) {
        spdlog::get(Systems::asset_loading)->info("starting to process nodes");
        recursively_process_nodes(scene->mRootNode, scene);
        spdlog::get(Systems::asset_loading)->info("processed all nodes");
    });

    return this->meshes;
};

void ModelLoader::process_function(aiMesh *mesh, const aiScene *scene) {}

/**
 * /note this function is guaranteed to terminate because all models are finite and non-cyclic
 */
std::function<void(aiNode *, const aiScene *)>
ModelLoader::recursively_process_nodes_closure(std::function<void(aiMesh *, const aiScene *)> process_function) {
    std::function<void(aiNode * node, const aiScene *scene)> recursively_process_nodes =
        [&recursively_process_nodes, process_function](aiNode *node, const aiScene *scene) {
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
