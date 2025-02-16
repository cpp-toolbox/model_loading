#ifndef MODEL_LOADING_HPP
#define MODEL_LOADING_HPP

#include <glm/glm.hpp>
#include <string>
#include <utility>
#include <vector>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <functional>

#include "sbpt_generated_includes.hpp"

enum TextureType { DIFFUSE, SPECULAR };

struct TextureInfo {
    TextureType type;
    std::string path;
};

std::vector<draw_info::IndexedVertexPositions> parse_model_into_ivps(const std::string &model_path);
std::vector<draw_info::IVPTextured> parse_model_into_ivpts(const std::string &model_path, bool flip_uvs);
std::vector<draw_info::IVPNTextured> parse_model_into_ivpnts(const std::string &model_path, bool flip_uvs);

std::string get_directory_of_asset(const std::string &asset_path);

class RecIvpntCollector {
  public:
    RecIvpntCollector(const std::string &model_path);
    std::string directory_to_model;
    std::vector<draw_info::IVPNTextured> ivpnts;
    void rec_process_nodes(aiNode *node, const aiScene *scene);
};

class RecIvptCollector {
  public:
    RecIvptCollector(const std::string &model_path);
    std::string directory_to_model;
    std::vector<draw_info::IVPTextured> ivpts;
    void rec_process_nodes(aiNode *node, const aiScene *scene);
};

class RecIvpCollector {
  public:
    std::vector<draw_info::IndexedVertexPositions> ivps;
    void rec_process_nodes(aiNode *node, const aiScene *scene);
};

draw_info::IndexedVertexPositions process_mesh_ivps(aiMesh *mesh, const aiScene *scene);
draw_info::IVPTextured process_mesh_ivpts(aiMesh *mesh, const aiScene *scene, const std::string &directory_to_model);
draw_info::IVPNTextured process_mesh_ivpnts(aiMesh *mesh, const aiScene *scene, const std::string &directory_to_model);
std::vector<unsigned int> process_mesh_indices(aiMesh *mesh);

std::vector<TextureInfo> process_mesh_materials(aiMesh *mesh, const aiScene *scene,
                                                const std::string &directory_to_asset_being_loaded = "");

std::vector<TextureInfo> get_texture_info_for_material(aiMaterial *material, aiTextureType type,
                                                       TextureType texture_type,
                                                       const std::string &directory_to_asset_being_loaded = "");

std::vector<glm::vec3> process_mesh_vertex_positions(aiMesh *mesh);
std::vector<glm::vec2> process_mesh_texture_coordinates(aiMesh *mesh);
std::vector<glm::vec3> process_mesh_normals(aiMesh *mesh);

glm::vec3 assimp_to_glm_3d_vector(aiVector3D assimp_vector);
std::vector<glm::vec3> get_ordered_vertex_positions(std::vector<glm::vec3> &vertex_positions,
                                                    std::vector<unsigned int> &indices);

#endif // MODEL_LOADING_HPP
