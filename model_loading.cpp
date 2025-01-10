#include <spdlog/spdlog.h>
#include <assimp/postprocess.h>
#include <filesystem>

#include "model_loading.hpp"

glm::vec3 assimp_to_glm_3d_vector(aiVector3D assimp_vector) {
    return {assimp_vector.x, assimp_vector.y, assimp_vector.z};
}

std::vector<IndexedVertexPositions> parse_model_into_ivps(const std::string &model_path) {
    Assimp::Importer importer;
    const aiScene *scene =
        importer.ReadFile(model_path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Error: Assimp - " << importer.GetErrorString() << std::endl;
    }
    RecIvpCollector ric;
    ric.rec_process_nodes(scene->mRootNode, scene);
    return ric.ivps;
}

std::vector<IVPTextured> parse_model_into_ivpts(const std::string &model_path, bool flip_uvs) {
    Assimp::Importer importer;

    unsigned int flags = aiProcess_Triangulate | aiProcess_CalcTangentSpace;
    if (flip_uvs) {
        flags |= aiProcess_FlipUVs;
    }

    const aiScene *scene = importer.ReadFile(model_path, flags);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Error: Assimp - " << importer.GetErrorString() << std::endl;
    }
    RecIvptCollector ric(model_path);
    ric.rec_process_nodes(scene->mRootNode, scene);
    return ric.ivpts;
}

std::vector<IVPNTextured> parse_model_into_ivpnts(const std::string &model_path, bool flip_uvs) {
    Assimp::Importer importer;

    unsigned int flags = aiProcess_Triangulate | aiProcess_CalcTangentSpace;
    if (flip_uvs) {
        flags |= aiProcess_FlipUVs;
    }

    const aiScene *scene = importer.ReadFile(model_path, flags);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Error: Assimp - " << importer.GetErrorString() << std::endl;
    }
    RecIvpntCollector ric(model_path);
    ric.rec_process_nodes(scene->mRootNode, scene);
    return ric.ivpnts;
}

/// @brief Extracts the directory path of a given asset path using std::filesystem.
/// @param asset_path The full path of the asset.
/// @return The directory part of the asset path.
std::string get_directory_of_asset(const std::string &asset_path) {
    std::filesystem::path path(asset_path);
    // Convert parent_path to string and append the preferred separator
    return path.parent_path().string() + std::string(1, std::filesystem::path::preferred_separator);
}

void RecIvpCollector::rec_process_nodes(aiNode *node, const aiScene *scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        unsigned int mesh_index = node->mMeshes[i];
        aiMesh *mesh = scene->mMeshes[mesh_index];
        this->ivps.push_back(process_mesh_ivps(mesh, scene));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        this->rec_process_nodes(node->mChildren[i], scene);
    }
}

void RecIvptCollector::rec_process_nodes(aiNode *node, const aiScene *scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        unsigned int mesh_index = node->mMeshes[i];
        aiMesh *mesh = scene->mMeshes[mesh_index];
        this->ivpts.push_back(process_mesh_ivpts(mesh, scene, directory_to_model));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        this->rec_process_nodes(node->mChildren[i], scene);
    }
}

RecIvptCollector::RecIvptCollector(const std::string &model_path) {
    directory_to_model = get_directory_of_asset(model_path);
}

RecIvpntCollector::RecIvpntCollector(const std::string &model_path) {
    directory_to_model = get_directory_of_asset(model_path);
    std::cout << "model path: " << model_path << " directory to model: " << directory_to_model << std::endl;

}

void RecIvpntCollector::rec_process_nodes(aiNode *node, const aiScene *scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        unsigned int mesh_index = node->mMeshes[i];
        aiMesh *mesh = scene->mMeshes[mesh_index];
        this->ivpnts.push_back(process_mesh_ivpnts(mesh, scene, directory_to_model));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        this->rec_process_nodes(node->mChildren[i], scene);
    }
}

IndexedVertexPositions process_mesh_ivps(aiMesh *mesh, const aiScene *scene) {
    std::vector<glm::vec3> vertices = process_mesh_vertex_positions(mesh);
    std::vector<unsigned int> indices = process_mesh_indices(mesh);
    return {indices, vertices};
};

IVPTextured process_mesh_ivpts(aiMesh *mesh, const aiScene *scene, const std::string &directory_to_model) {
    std::vector<glm::vec3> vertices = process_mesh_vertex_positions(mesh);
    std::vector<unsigned int> indices = process_mesh_indices(mesh);
    std::vector<glm::vec2> texture_coordinates = process_mesh_texture_coordinates(mesh);
    std::vector<TextureInfo> texture_data = process_mesh_materials(mesh, scene, directory_to_model);
    std::string main_texture = texture_data[0].path;
    std::filesystem::path fs_path = main_texture;
    // Convert to the preferred format for the operating system
    std::string texture_native_path = fs_path.make_preferred().string();
    return {indices, vertices, texture_coordinates, texture_native_path};
};

IVPNTextured process_mesh_ivpnts(aiMesh *mesh, const aiScene *scene, const std::string &directory_to_model) {
    std::vector<glm::vec3> vertices = process_mesh_vertex_positions(mesh);
    std::vector<glm::vec3> normals = process_mesh_normals(mesh);
    std::vector<unsigned int> indices = process_mesh_indices(mesh);
    std::vector<glm::vec2> texture_coordinates = process_mesh_texture_coordinates(mesh);
    std::vector<TextureInfo> texture_data = process_mesh_materials(mesh, scene, directory_to_model);
    std::string main_texture = texture_data[0].path;
    std::filesystem::path fs_path = main_texture;
    // Convert to the preferred format for the operating system
    std::string texture_native_path = fs_path.make_preferred().string();
    return {indices, vertices, normals, texture_coordinates, texture_native_path};
};

std::vector<glm::vec3> process_mesh_vertex_positions(aiMesh *mesh) {
    std::vector<glm::vec3> vertices;
    /*spdlog::get(Systems::asset_loading)->info("This mesh has {} vertex_positions", mesh->mNumVertices);*/
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        glm::vec3 vertex = {assimp_to_glm_3d_vector(mesh->mVertices[i])};
        vertices.push_back(vertex);
    }
    return vertices;
}

std::vector<TextureInfo> process_mesh_materials(aiMesh *mesh, const aiScene *scene,
                                                const std::string &directory_to_asset_being_loaded) {
    std::vector<TextureInfo> textures;

    std::cout << "processing mesh materials: " << directory_to_asset_being_loaded << std::endl;

    bool mesh_has_materials = mesh->mMaterialIndex >= 0;

    if (mesh_has_materials) {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

        std::vector<TextureInfo> diffuse_maps_texture_info = get_texture_info_for_material(
            material, aiTextureType_DIFFUSE, TextureType::DIFFUSE, directory_to_asset_being_loaded);
        textures.insert(textures.end(), diffuse_maps_texture_info.begin(), diffuse_maps_texture_info.end());

        for (auto &texture : textures) {
            std::cout << texture.path << std::endl;
        }

        if (diffuse_maps_texture_info.size() == 0) {
            /*spdlog::get(Systems::asset_loading)->warn("This material doesn't have any diffuse maps");*/
        }

        std::vector<TextureInfo> specular_maps_texture_info = get_texture_info_for_material(
            material, aiTextureType_SPECULAR, TextureType::SPECULAR, directory_to_asset_being_loaded);
        textures.insert(textures.end(), specular_maps_texture_info.begin(), specular_maps_texture_info.end());

        if (specular_maps_texture_info.size() == 0) {
            /*spdlog::get(Systems::asset_loading)->info("This material doesn't have any specular maps");*/
        }
    }
    return textures;
}

std::vector<TextureInfo> get_texture_info_for_material(aiMaterial *material, aiTextureType type,
                                                       TextureType texture_type,
                                                       const std::string &directory_to_asset_being_loaded) {
    std::vector<TextureInfo> textures;

    // loop through the textures of the specified type
    for (unsigned int i = 0; i < material->GetTextureCount(type); i++) {
        aiString texture_path;
        material->GetTexture(type, i, &texture_path);

        // construct the asset path
        std::string asset_path = directory_to_asset_being_loaded + std::string(texture_path.C_Str());

        // create the texture info and add it to the vector
        TextureInfo texture{texture_type, asset_path.c_str()};
        textures.push_back(texture);
    }

    // if no diffuse textures are found, add the missing texture
    if (textures.empty() && texture_type == TextureType::DIFFUSE) {
        std::cout << "no messages" << std::endl;
        std::string missing_texture_path = "assets/images/missing_texture.png";
        TextureInfo missing_texture{texture_type, missing_texture_path.c_str()};
        textures.push_back(missing_texture);
    }

    return textures;
}

std::vector<glm::vec2> process_mesh_texture_coordinates(aiMesh *mesh) {
    std::vector<glm::vec2> texture_coordinates;
    // there is a better way to do this at some point
    bool mesh_has_texture_coordinates = mesh->mTextureCoords[0] != nullptr;
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {

        glm::vec2 texture_coordinate;
        if (mesh_has_texture_coordinates) {
            texture_coordinate = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        } else {
            /*spdlog::get(Systems::asset_loading)->warn("This mesh doesn't have texture coordinates!");*/
            texture_coordinate = glm::vec2(0.0f, 0.0f);
        }
        texture_coordinates.push_back(texture_coordinate);
    }
    return texture_coordinates;
}

std::vector<glm::vec3> process_mesh_normals(aiMesh *mesh) {
    std::vector<glm::vec3> normals;
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        if (mesh->HasNormals()) {
            normals.push_back(assimp_to_glm_3d_vector(mesh->mNormals[i]));
        } else {
            /*spdlog::get(Systems::asset_loading)->warn("This mesh doesn't have texture coordinates!");*/
            normals.push_back(glm::vec3(0, 0, 0));
        }
    }
    return normals;
}

std::vector<unsigned int> process_mesh_indices(aiMesh *mesh) {
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
