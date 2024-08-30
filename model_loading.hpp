#ifndef MODEL_LOADING_HPP
#define MODEL_LOADING_HPP

#include "glm/glm.hpp"
#include <string>
#include <utility>
#include <vector>
#include <GLFW/glfw3.h>
#include <assimp/scene.h>
#include <functional>

glm::vec3 assimp_to_glm_3d_vector(aiVector3D assimp_vector);
std::vector<glm::vec3> get_ordered_vertex_positions(std::vector<glm::vec3> &vertex_positions, std::vector<unsigned int> &indices);

/**
 * \brief A mesh is a collection of vertex_positions and indices which describe the order in which the vertex_positions
 * should be traversed in order to draw it using the standard interpretation of 3 sequential vertex_positions as a
 * triangle.
 */
class Mesh {
  public:
    Mesh(std::vector<glm::vec3> &vertex_positions, std::vector<unsigned int> &indices)
        : vertex_positions(vertex_positions), indices(indices){};
    std::vector<unsigned int> indices;
    std::vector<glm::vec3> vertex_positions;
};

/**
 * /brief A model is a collection of Meshes
 */

class Model {
    public: 
        Model(std::vector<Mesh> meshes): meshes(std::move(meshes)) {};
        std::vector<Mesh> meshes;
        std::vector<std::vector<glm::vec3>> get_ordered_vertex_positions_for_each_mesh();
};

class ModelLoader {
    std::vector<Mesh> meshes;
    std::string path_to_assets_directory = "assets";
    void process_function(aiMesh *mesh, const aiScene *scene);
    std::vector<glm::vec3> process_mesh_vertex_positions(aiMesh *mesh);
    std::vector<unsigned int> process_mesh_indices(aiMesh *mesh);

  public:
    [[nodiscard]] Model load_model(const std::string &path);

  protected:
    std::string directory_to_asset_being_loaded;
    Mesh process_mesh(aiMesh *mesh, const aiScene *scene);
    std::function<void(aiNode *, const aiScene *)>
    recursively_process_nodes_closure(std::function<void(aiMesh *, const aiScene *)> process_function);
    template <typename func> void call_function_with_assimp_importer_context(const std::string &path, func fn);
};

#include "model_loading.tpp"

#endif // MODEL_LOADING_HPP
