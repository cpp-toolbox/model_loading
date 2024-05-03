#include "model_loading.hpp"

#include <iostream>

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include <utility>

/**
 * overview:
 * initialization:
 *  first we iterate through the entire assimp imported object recursively
 *  we store all the images we found in the base model, this is the initialization phase.
 *
 */

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices) {
    this->vertices = std::move(vertices);
    this->indices = std::move(indices);
};

/**
 * \brief loads a 3d model into wrapper structure
 *
 * \pre the shader files specified exist and are at this file path
 * \todo have a resource directory so that we don't have to specify a long relative path
 *
 * @param path the path to the model we want to load
 */
Model::Model(std::string path) { this->load_model(std::move(path)); }

/**
 * notes:
 * 	- this function is the entry point to the initialization process
 * 	- although this function looks short and simple, the call to process_node
 * 	is recursive and does all the work of parsing assimp's structure
 */
void Model::load_model(std::string path) {
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASIMP::" << importer.GetErrorString() << std::endl;
        return;
    }
    this->directory = path.substr(0, path.find_last_of("/"));
    printf("starting to process nodes \n");
    this->process_node(scene->mRootNode, scene);
    printf("processed all nodes\n");
};

/**
 * notes:
 * - recall that a node may contains a collection of meshes, and
 * also children nodes
 * - this function is guarenteed to terminate because modellers
 * create strctures made up of finitely many nodes and meshes.
 * - this can be thought of the same way that each folder in blender can have individual meshes inside or more
 * folders (I believe a folder is called a collection) and just represents arbitrary nesting
 */
void Model::process_node(aiNode *node, const aiScene *scene) {
    printf("stared processing meshes\n");
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        this->meshes.push_back(this->process_mesh(mesh, scene));
    }
    printf("finished processing meshes\n");
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        process_node(node->mChildren[i], scene);
    }
};

glm::vec3 assimp_to_glm_3d_vector(aiVector3D assimp_vector) {
    return {assimp_vector.x, assimp_vector.y, assimp_vector.z};
}

std::vector<Vertex> Model::process_mesh_vertices(aiMesh *mesh) {
    std::vector<Vertex> vertices;

    bool mesh_has_texture_coordinates = mesh->mTextureCoords[0] != nullptr;
    printf("This mesh has %d vertices\n", mesh->mNumVertices);
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        vertex.position = assimp_to_glm_3d_vector(mesh->mVertices[i]);
        vertices.push_back(vertex);
    }
    return vertices;
}

/**
 * \brief given a mesh iterate through each face storing all indices of each vertex on that face
 */
std::vector<unsigned int> Model::process_mesh_indices(aiMesh *mesh) {
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

/**
 * description
 * 	takes in assimp's interpretation of a mesh, and then parses it
 * 	into our version of a mesh
 */
Mesh Model::process_mesh(aiMesh *mesh, const aiScene *scene) {
    std::vector<Vertex> vertices = this->process_mesh_vertices(mesh);
    std::vector<unsigned int> indices = this->process_mesh_indices(mesh);
    return {vertices, indices};
};
