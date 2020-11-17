#pragma once

#include <string>
#include <math.h>
#include <chrono>
#include "torus.h"
#include <glm/gtc/constants.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


class Map {

    typedef std::chrono::time_point<std::chrono::high_resolution_clock> Time;

    private:
    
    glm::vec2 position = glm::vec2(750, 150);
    glm::vec2 direction = glm::vec2(0, 1);

    float alpha = 0.0f;
    float speed = 0.0f;

    Torus torus;

    Time prev_time;

    int time_delta = 24370543;


    public:
    
    Map(const Torus& torus, Time start_time) : torus(torus) , prev_time(start_time) {}


    int move(Time current_time) {

        int d_time = std::chrono::duration_cast<std::chrono::nanoseconds>(current_time - prev_time).count();

        position += glm::vec2(direction[0], direction[1]) * (speed / time_delta * d_time);
        prev_time = current_time;
    
        if (position[0] >= torus.get_y_count()) {
            position[0] = 0;
        }
        if (position[0] < 0) {
            position[0] = torus.get_y_count();
        }
        if (position[1] >= torus.get_x_count()) {
            position[1] = 0;
        }
        if (position[1] < 0) {
            position[1] = torus.get_x_count();
        }

        return d_time;
    }


    glm::mat4 get_rotation_matrix() {
        return glm::rotate(alpha, glm::vec3(1, 0, 0));
    }

    float get_angle() {
        return alpha;
    }

    int get_time_delta() {
        return time_delta;
    }


    glm::vec3 get_torus_dir_x() {
        auto pos_x = get_point_on_torus()[0];
        auto pos_y = get_point_on_torus()[1];
        glm::vec3 point1 = torus.get_vertex(pos_x, pos_y);
        glm::vec3 point2 = torus.get_vertex(pos_x + 1, pos_y);

        return point2 - point1;
    }


    glm::vec3 get_torus_dir_y() {
        auto pos_x = get_point_on_torus()[0];
        auto pos_y = get_point_on_torus()[1];
        glm::vec3 point1 = torus.get_vertex(pos_x, pos_y);
        glm::vec3 point2 = torus.get_vertex(pos_x, pos_y + 1);

        return point2 - point1;
    }


    glm::vec3 get_torus_dir() {
        auto pos_x = get_point_on_torus()[0];
        auto pos_y = get_point_on_torus()[1];
        glm::vec3 point1 = torus.get_vertex(pos_x, pos_y);
        glm::vec3 point2 = torus.get_vertex(pos_x + 2.f * direction[0], pos_y + 2.f * direction[1]);

        return point2 - point1;
    }

    glm::vec2 & get_point_on_torus() {
        return position;
    }

    glm::vec2 get_direction_on_torus() {
        return glm::normalize(direction);
    }

    void buttons_callback() {
        if (ImGui::IsKeyDown(GLFW_KEY_UP)) {
            speed = -1.0f;
        }
        else if (ImGui::IsKeyDown(GLFW_KEY_DOWN)) {
            speed = 1.0f;
        } else {
            speed = 0;
        }
        if (ImGui::IsKeyDown(GLFW_KEY_RIGHT)) {
            alpha += speed * 0.03f;
        }
        if (ImGui::IsKeyDown(GLFW_KEY_LEFT)) {
            alpha -= speed * 0.03f;
        }
        if (abs(alpha) > 2 * 3.14) {
            alpha = 0;
        }
        direction = glm::vec2(cos(alpha), sin(alpha));
    }
};