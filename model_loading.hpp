#ifndef MODEL_LOADING_HPP
#define MODEL_LOADING_HPP

#include "glm/glm.hpp"
#include <string>
#include <vector>
#include <assimp/scene.h>

/**
 * description:
 * 	a vertex in the context of a 3d model
 */
struct Vertex {
    glm::vec3 position;
};

/**
 * description:
 * 	a collection of vertices along with an ordering of how to traverse them
 *
 * example:
 * 	a simple hamburger 3d model might have 3 meshes, one for the bottom bun,
 * the burger and the top bun
 *
 */
class Mesh {
  public:
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices);
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
};

/**
 * description:
 *  a collection of meshes, provides the ability to load from file
 * example:
 *  a 3d model that represents the entire burger as in the mesh example
 */
class Model {
  public:
    // Should the shader be stored inside of the model class? Unique to each
    // shader?
    Model(std::string path);
    std::vector<Mesh> meshes;

  private:
    void load_model(std::string path);

    std::string directory;

    void process_node(aiNode *node, const aiScene *scene);

    std::vector<Vertex> process_mesh_vertices(aiMesh *mesh);
    std::vector<unsigned int> process_mesh_indices(aiMesh *mesh);
    Mesh process_mesh(aiMesh *mesh, const aiScene *scene);
};

#endif // MODEL_LOADING_HPP
