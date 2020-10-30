#include <string>
#include <array> 
#include <vector>
#include <cmath>
#include "opengl_shader.h"
#include "textures.h"

#include <iostream>
using namespace std;


class Torus
{
    private:

    const size_t x_count = 300;
    const size_t y_count = 300 * 5;

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


    size_t get_vertices_count() {
        return x_count * y_count;
    }

    float get_vertex_height(size_t i, size_t j) {
        size_t ii = i >= y_count / 2 ? y_count - i - 1 : i;
        size_t jj = j >= x_count / 2 ? x_count - j - 1 : j;
        int y = (1.0 * ii / y_count) * map_height;
        int x = (1.0 * jj / x_count) * map_width;
        return 1.0 * height_map[3*(y * map_width + x)] / 255.0 * r*2;
    }


    std::vector<float> get_vertices() {
        std::vector<float> result;
        for (size_t i = 0; i < y_count; i++) {
            for (size_t j = 0; j < x_count; j++) {

                float phi = i == y_count - 1 ? 0 : 2 * M_PI / (y_count-1) * i;
                float psi = j == x_count - 1 ?  -M_PI : 2 * M_PI / (x_count - 1) * j - M_PI;

                float h = get_vertex_height(i, j);
            
                // h = 0;
                float x = (R + (r + h) * cos(psi)) * cos(phi);
                float y = (R + (r + h) * cos(psi)) * sin(phi);
                float z = (r + h) * sin(psi);

                result.push_back(x);
                result.push_back(y);
                result.push_back(z);
            }
        }
        return result;
    }

    std::vector<float> get_normals() {
        std::vector<float> result;
         for (size_t i = 0; i < y_count; i++) {
            for (size_t j = 0; j < x_count; j++) {
                result.push_back(get_vertex_height(i, j));
                result.push_back(get_vertex_height(i, j));
                result.push_back(get_vertex_height(i, j));
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
