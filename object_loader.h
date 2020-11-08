#pragma once
#include <string>
#include <vector>
#include <iostream>
#define TINYOBJLOADER_IMPLEMENTATION 
#include "tiny_obj_loader.h"
#include "opengl_shader.h"
using namespace std;


class Object {

  private:

  float object_scale = 0.005;

  size_t vertices_count;

  GLuint vbo;
  GLuint vao;
  GLuint ebo;

  float min_x = std::numeric_limits<float>::max();
  float min_y = std::numeric_limits<float>::max();
  float min_z = std::numeric_limits<float>::max();

  float max_x = std::numeric_limits<float>::min();
  float max_y = std::numeric_limits<float>::min();
  float max_z = std::numeric_limits<float>::min();

  public:

  Object(
    std::vector<float>& vertices,
    std::vector<float>& normals,
    std::vector<float>& colors,
    std::vector<unsigned int>& vertices_indices
  ) {
        GLuint vbo, vao, ebo;

        vertices_count = vertices_indices.size();
        
        float triangle_vertices[vertices_indices.size() * 8] = { 0 };
        unsigned int triangle_indices[vertices_count] = { 0 };

        for (size_t i = 0, j = 0, k = 0; k < vertices_indices.size() * 8; i += 3, j += 2, k+=8) {
            triangle_vertices[k] = vertices[i];
            triangle_vertices[k + 1] = vertices[i + 1];
            triangle_vertices[k + 2] = vertices[i  + 2];
 
            triangle_vertices[k + 3] = normals[i];
            triangle_vertices[k + 4] = normals[i + 1];
            triangle_vertices[k + 5] = normals[i  + 2];

            triangle_vertices[k + 6] = colors[j];
            triangle_vertices[k + 7] = colors[j + 1];
        }

        for (size_t i = 0; i < vertices_indices.size(); i++) {
            triangle_indices[i] = vertices_indices[i];
        } 

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_vertices), triangle_vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangle_indices), triangle_indices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        this->vbo = vbo;
        this->vao = vao;
        this->ebo = ebo;

        compute_min_max(vertices);
  }


  void compute_min_max(const std::vector<float>& vertices) {
      for (size_t i = 0; i < vertices.size(); i+=3) {
          float x = vertices[i];
          float y = vertices[i + 1];
          float z = vertices[i + 2];
          min_x = min(min_x, x);
          min_y = min(min_y, y);
          min_z = min(min_z, z);
          max_x = max(max_x, x);
          max_y = max(max_y, y);
          max_z = min(max_z, z);
      }
  }

  glm::vec3 get_center() {
      return {
          (max_x + min_x) / 2,
          (max_y + min_y) / 2, 
          (max_z + min_z) / 2
      };
  }

  glm::vec3 get_bottom_center() {
      return {
          (max_x + min_x) / 2,
          (max_y + min_y) / 2, 
          min_z
      };
  }

  float get_bottom_height() {
     return -min_y * object_scale;
  }

  float get_scale() {
      return object_scale;
  }

  void render(shader_t& shader, GLuint texture, GLuint cubemap_texture) {

        glActiveTexture(GL_TEXTURE0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, texture);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture);

        shader.use();
        glBindVertexArray(vao);

        glDrawElements(GL_TRIANGLES, vertices_count, GL_UNSIGNED_INT, 0);
  }

  glm::mat4 get_model_matrix() {
      return glm::rotate(-3.14f / 2.f, glm::vec3(0, 0, 1)) *
             glm::rotate(-3.14f / 2.f, glm::vec3(1, 0, 0)) * 
             glm::rotate(3.14f, glm::vec3(0, 0, 1)) * 
             glm::scale(glm::vec3(object_scale, object_scale, object_scale));
  }
};


class ObjLoader {

    private:

    ObjLoader() { }

    public:

    static Object load(const std::string& path, const std::string& file) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string err;

        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, file.c_str(), path.c_str());

        if (!err.empty()) {
            std::cerr << err << std::endl;
        }

        if (!ret) {
            exit(1);
        }

        std::vector<float> vertices;
        std::vector<float> normals;
        std::vector<float> colors;
        std::vector<unsigned int> vertices_indices;

        for (auto const & shape : shapes) {
            for (size_t i = 0; i < shape.mesh.indices.size(); i++) {
                auto idx = shape.mesh.indices[i];
                
                vertices_indices.push_back(i);

                vertices.push_back(attrib.vertices[3 * idx.vertex_index + 0]); 
                vertices.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
                vertices.push_back(attrib.vertices[3 * idx.vertex_index + 2]);
                
                normals.push_back(attrib.normals[idx.normal_index * 3 + 0]);
                normals.push_back(attrib.normals[idx.normal_index * 3 + 1]);
                normals.push_back(attrib.normals[idx.normal_index * 3 + 2]);

                colors.push_back(attrib.texcoords[2*idx.texcoord_index + 0]);
                colors.push_back(-attrib.texcoords[2*idx.texcoord_index + 1]);                
            }
        }
  
       return Object(vertices, normals, colors, vertices_indices);
    }
};