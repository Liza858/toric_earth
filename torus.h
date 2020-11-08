#pragma once

#include <string>
#include <array> 
#include <vector>
#include <tuple>
#include <cmath>
#include "opengl_shader.h"
#include "textures.h"


class Torus
{
    private:

    const size_t x_count = 300;
    const size_t y_count = 300 * 5;

    const float torus_scale = 1.f;

    float R;
    float r;
    unsigned char *height_map;
    std::array<Texture, 3> torus_textures;
    std::array<Texture, 3> detail_textures;

    int map_width = 0;
    int map_height = 0;

    size_t indices_count;

    GLuint vbo;
    GLuint vao;
    GLuint ebo;

    float get_phi(float i) {
        return i >= y_count - 1 ? 0 : 2 * M_PI / (y_count - 1) * i;
    }

    float get_psi(float j) {
        return j >= x_count - 1 ?  -M_PI : 2 * M_PI / (x_count - 1) * j - M_PI;
    }

    std::vector<float> get_normals() {
        std::vector<float> result;
         for (size_t i = 0; i < y_count; i++) {
            for (size_t j = 0; j < x_count; j++) {

                auto n = get_normal(i, j);

                result.push_back(n[0]);
                result.push_back(n[1]);
                result.push_back(n[2]);
            }
         }
        return result;
    }

    std::vector<float> get_tex_coors() {
        std::vector<float> result;
        for (size_t i = 0; i < y_count; i++) {
            for (size_t j = 0; j < x_count; j++) {
                float tex_x = 1.0 * j / (x_count - 1);
                float tex_y = 1.0 * i / (y_count - 1);
                result.push_back(tex_x);
                result.push_back(tex_y);
                result.push_back(get_vertex_height(i, j));
            }
        }
        return result;
    }

    std::vector<unsigned int> get_indices() {
        std::vector<unsigned int> result;
        for (size_t i = 0; i < y_count - 1; i++) {
            for (size_t j = 0; j < x_count - 1; j++) {

                size_t ii = i == y_count - 1 ? 0 : i + 1;
                size_t jj = j == x_count - 1 ? 0 : j + 1;

                result.push_back(i*x_count + j);
                result.push_back(i*x_count + jj);
                result.push_back(ii*x_count + j);
                result.push_back(i*x_count + jj);
                result.push_back(ii*x_count + j);
                result.push_back(ii*x_count + jj);
            }
        }

        return result;
    }

    std::vector<float> get_vertices() {
        std::vector<float> result;
        for (size_t i = 0; i < y_count; i++) {
            for (size_t j = 0; j < x_count; j++) {
                glm::vec3 vertex = get_vertex(i, j);
                result.push_back(vertex[0]);
                result.push_back(vertex[1]);
                result.push_back(vertex[2]);
            }
        }
        return result;
    }

    glm::vec3 get_normal(glm::vec3&& v1, glm::vec3&& v2) {
        return cross(v2, v1);
    }

    glm::vec3 get_normal(
        glm::vec3& n1,
        glm::vec3& n2,
        glm::vec3& n3,
        glm::vec3& n4
    ) {
        return normalize(n1 + n2 + n3 + n4);
    }

    void load_height_map(const std::string& height_map_file) {
        int nrChannels = 0;
        height_map = stbi_load(
            height_map_file.c_str(),
            &map_width,
            &map_height,
            &nrChannels, 
            STBI_rgb
        );
        if (!height_map) {
            throw std::runtime_error("error in loading height map");
        }
    }

    public:

    Torus(
        float R,
        float r, 
        const std::string& height_map_file,
        const std::array<Texture, 3>& torus_textures,
        const std::array<Texture, 3>& detail_textures
    ) 
      : R(R) 
      , r(r)
      , height_map(height_map)
      , torus_textures(torus_textures)
      , detail_textures(detail_textures)
    {
       
        load_height_map(height_map_file);
       
        GLuint vbo, vao, ebo;

        std::vector<float> vertices = get_vertices();
        std::vector<float> normals = get_normals();
        std::vector<float> tex_coords = get_tex_coors();

        std::vector<float> triangle_vertices(get_vertices_count() * 9);
        std::vector<unsigned int> triangle_indices = get_indices();

        indices_count = triangle_indices.size();
    
        for (size_t i = 0, k = 0; k < get_vertices_count() * 9; i += 3, k += 9) {

            triangle_vertices[k] = vertices[i];
            triangle_vertices[k + 1] = vertices[i + 1];
            triangle_vertices[k + 2] = vertices[i + 2];
 
            triangle_vertices[k + 3] = normals[i];
            triangle_vertices[k + 4] = normals[i + 1];
            triangle_vertices[k + 5] = normals[i + 2];

            triangle_vertices[k + 6] = tex_coords[i];
            triangle_vertices[k + 7] = tex_coords[i + 1];
            triangle_vertices[k + 8] = tex_coords[i + 2];
        }


        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(float) * triangle_vertices.size(),
            triangle_vertices.data(),
            GL_STATIC_DRAW
        );
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            sizeof(unsigned int) * triangle_indices.size(),
            triangle_indices.data(),
            GL_STATIC_DRAW
        );
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        this->vbo = vbo;
        this->vao = vao;
        this->ebo = ebo;
    }


    size_t get_vertices_count() {
        return x_count * y_count;
    }

    float get_vertex_height(size_t i, size_t j) {
        return get_vertex_height(1.0f * i, 1.0f * j);
    }

    float get_vertex_height(float i, float j) {
        float ii = i >= y_count / 2 ? y_count - i - 1 : i;
        float jj = j >= x_count / 2 ? x_count - j - 1 : j;
        int y = floor((ii / y_count) * map_height);
        int x = floor((jj / x_count) * map_width);
        return 1.0 * height_map[3*(y * map_width + x)] / 255.0 * r; // % от r
    }

    glm::vec3 get_vertex(size_t i, size_t j) {
        float phi = i == y_count - 1 ? 0 : 2 * M_PI / (y_count - 1) * i;
        float psi = j == x_count - 1 ?  -M_PI : 2 * M_PI / (x_count - 1) * j - M_PI;
        float h = get_vertex_height(i, j);
    
        float x = (R + (r + h) * cos(psi)) * cos(phi);
        float y = (R + (r + h) * cos(psi)) * sin(phi);
        float z = (r + h) * sin(psi);

        return {x, y, z};
    }


    glm::vec3 get_vertex_without_height(float i, float j, float h = 0) {
        float phi = get_phi(i);
        float psi = get_psi(j);
    
        float x = (R + (r + h) * cos(psi)) * cos(phi);
        float y = (R + (r + h) * cos(psi)) * sin(phi);
        float z = (r + h) * sin(psi);

        return {x, y, z};
    }
    

    glm::vec3 get_vertex(float i, float j) {
        float phi = get_phi(i);
        float psi = get_psi(j);
        float h = get_vertex_height(i, j);
    
        float x = (R + (r + h) * cos(psi)) * cos(phi);
        float y = (R + (r + h) * cos(psi)) * sin(phi);
        float z = (r + h) * sin(psi);

        return {x, y, z};
    }


    glm::vec3 get_normal(size_t i, size_t j, bool with_heghts = true) {
        auto v = get_vertex(i, j);

        int i1 = i == 0 ? y_count - 2 : i - 1;
        int i2 = i == y_count - 1 ? 1 : i + 1;
        int j1 = j == 0 ? x_count - 2 : j - 1;
        int j2 = j == x_count - 1 ? 1 : j + 1;

        glm::vec3 a, b, c, d;
        if (with_heghts) {
            a = get_vertex(i2, j);
            b = get_vertex(i, j1);
            c = get_vertex(i1, j);
            d = get_vertex(i, j2);
        } else {
            a = get_vertex_without_height(i2, j);
            b = get_vertex_without_height(i, j1);
            c = get_vertex_without_height(i1, j);
            d = get_vertex_without_height(i, j2);
        }

        auto n1 = get_normal(
            { a[0] - v[0], a[1] - v[1], a[2] - v[2] },
            { b[0] - v[0], b[1] - v[1], b[2] - v[2] }
        );
        auto n2= get_normal(
            { b[0] - v[0], b[1] - v[1], b[2] - v[2] },
            { c[0] - v[0], c[1] - v[1], c[2] - v[2] }
        );
        auto n3 = get_normal(
            { c[0] - v[0], c[1] - v[1], c[2] - v[2] },
            { d[0] - v[0], d[1] - v[1], d[2] - v[2] }
        );
        auto n4 = get_normal(
            { d[0] - v[0], d[1] - v[1], d[2] - v[2] },
            { a[0] - v[0], a[1] - v[1], a[2] - v[2] }
        );

        return get_normal(n1, n2, n3, n4);
    }


    glm::vec3 get_normal(float i, float j, bool with_heghts = true) {
        float delta = 1;

        auto v = get_vertex(i, j);

        int i1 = i <= 0 ? y_count - 2 : i - delta;
        int i2 = i >= y_count - 1 ? 1 : i + delta;
        int j1 = j <= 0 ? x_count - 2 : j - delta;
        int j2 = j >= x_count - 1 ? 1 : j + delta;

        glm::vec3 a, b, c, d;
        if (with_heghts) {
            a = get_vertex(i2, j);
            b = get_vertex(i, j1);
            c = get_vertex(i1, j);
            d = get_vertex(i, j2);
        } else {
            a = get_vertex_without_height(i2, j);
            b = get_vertex_without_height(i, j1);
            c = get_vertex_without_height(i1, j);
            d = get_vertex_without_height(i, j2);
        }

        auto n1 = get_normal(
            { a[0] - v[0], a[1] - v[1], a[2] - v[2] },
            { b[0] - v[0], b[1] - v[1], b[2] - v[2] }
        );
        auto n2 = get_normal(
            { b[0] - v[0], b[1] - v[1], b[2] - v[2] },
            { c[0] - v[0], c[1] - v[1], c[2] - v[2] }
        );
        auto n3 = get_normal(
            { c[0] - v[0], c[1] - v[1], c[2] - v[2] },
            { d[0] - v[0], d[1] - v[1], d[2] - v[2] }
        );
        auto n4 = get_normal(
            { d[0] - v[0], d[1] - v[1], d[2] - v[2] },
            { a[0] - v[0], a[1] - v[1], a[2] - v[2] }
        );

        return get_normal(n1, n2, n3, n4);
    }


    float get_x_count() {
        return x_count;
    }


    float get_y_count() {
        return y_count;
    }


    glm::mat4 get_model_matrix() {
        return glm::scale(glm::vec3(torus_scale, torus_scale, torus_scale));
    }

    
    glm::mat4 get_translation_matrix(glm::vec2 position, Object obj) {
        return glm::rotate(get_phi(position[0]), glm::vec3(0, 0, 1)) *
               glm::translate(glm::vec3(R * torus_scale, 0, 0)) *
               glm::rotate(-get_psi(position[1]), glm::vec3(0, 1, 0)) *
               glm::translate(glm::vec3(r * torus_scale + get_vertex_height(position[0], position[1]), 0, 0));
    }

   
    void render(shader_t& torus_shader) {

        for (int i = 0; i < 3; i++) {
            glActiveTexture(GL_TEXTURE0 + torus_textures[i].get_id());
            glBindTexture(GL_TEXTURE_2D, torus_textures[i].get_id());
        }

         for (int i = 0; i < 3; i++) {
            glActiveTexture(GL_TEXTURE0 + detail_textures[i].get_id());
            glBindTexture(GL_TEXTURE_2D, detail_textures[i].get_id());
        }

        torus_shader.use();

        torus_shader.set_uniform("tex1", (int) torus_textures[0].get_id());
        torus_shader.set_uniform("tex2", (int) torus_textures[1].get_id());
        torus_shader.set_uniform("tex3", (int) torus_textures[2].get_id());

        torus_shader.set_uniform("detail_tex1", (int) detail_textures[0].get_id());
        torus_shader.set_uniform("detail_tex2", (int) detail_textures[1].get_id());
        torus_shader.set_uniform("detail_tex3", (int) detail_textures[2].get_id());

        glBindVertexArray(vao);

        glDrawElements(GL_TRIANGLES, indices_count, GL_UNSIGNED_INT, 0);
    }
};
