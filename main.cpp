#include <iostream>
#include <vector>
#include <chrono>
#include <utility>
	

#include <fmt/format.h>

#include <GL/glew.h>

// Imgui + bindings
#include "imgui.h"
#include "bindings/imgui_impl_glfw.h"
#include "bindings/imgui_impl_opengl3.h"

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

// Math constant and routines for OpenGL interop
#include <glm/gtc/constants.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <chrono>
#include <unistd.h>

#include "opengl_shader.h"
#include "environment.h"
#include "textures.h"
#include "object_loader.h"
#include "torus.h"
#include "map.h"
#include "shadow_map.h"


float mouse_offset_x = 0.0;
float mouse_offset_y = 0.0;

float angle_x = 0.0;
float angle_y = 0.0;

int zoom_sensitivity = 10;
float zoom = 1;

double mouse_prev_x = 0.0;
double mouse_prev_y = 0.0;

bool button_is_pressed = false;

float detail_coef = 2.1;
float detail_dist = 3.0;
int detail_repeat_count = 80;
int tex1_repeat_count = 3;
int tex2_repeat_count = 6;
int tex3_repeat_count = 6;
float spring_coef = 0.15;

int enable = 1;


static void glfw_error_callback(int error, const char *description)
{
   std::cerr << fmt::format("Glfw Error {}: {}\n", error, description);
}


int main(int, char **)
{
   // Use GLFW to create a simple window
   glfwSetErrorCallback(glfw_error_callback);
   if (!glfwInit())
      return 1;


   // GL 3.3 + GLSL 330
   const char *glsl_version = "#version 330";
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

   // Create window with graphics context
   GLFWwindow *window = glfwCreateWindow(1280, 720, "Dear ImGui - Conan", NULL, NULL);
   if (window == NULL)
      return 1;
   glfwMakeContextCurrent(window);
   glfwSwapInterval(1); // Enable vsync

   // Initialize GLEW, i.e. fill all possible function pointers for current OpenGL context
   if (glewInit() != GLEW_OK)
   {
      std::cerr << "Failed to initialize OpenGL loader!\n";
      return 1;
   }


   shader_t env_shader("environment.vs", "environment.fs");
   shader_t torus_shader("torus.vs", "torus.fs");
   shader_t obj_shader("obj.vs", "obj.fs");
   shader_t shadow_shader("shadow.vs", "shadow.fs");

   std::array<std::string, 6> env_textures = {
      "../environment/space1.jpg",
      "../environment/space1.jpg",
      "../environment/space1.jpg",
      "../environment/space1.jpg",
      "../environment/space1.jpg",
      "../environment/space1.jpg"
   };

   GLuint cubemap_texture = CubemapTextureLoader::load(env_textures);
   GLuint obj_textute = TextureLoader::load("../objects/Lexus.jpg");

   Object obj = ObjLoader::load("../objects/", "../objects/lexus_hs.obj");

   Environment env;

   std::array<Texture, 3> torus_textures = {
      Texture("../textures/tex8.jpg"),
      Texture("../textures/tex10.jpg"),
      Texture("../textures/tex11.jpg"),
   };

    std::array<Texture, 3> detail_textures = {
      Texture("../textures/detail1.jpg"),
      Texture("../textures/detail1.jpg"),
      Texture("../textures/detail1.jpg"),
   };


   Torus torus(
      10,
      2,
      "../maps/height_map.png",
      torus_textures,
      detail_textures
   );

   Shadow_map near_shadow_map;
   Shadow_map far_shadow_map;
   Shadow_map object_shadow_map;


   // Setup GUI context
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGuiIO &io = ImGui::GetIO();
   ImGui_ImplGlfw_InitForOpenGL(window, true);
   ImGui_ImplOpenGL3_Init(glsl_version);
   ImGui::StyleColorsDark();


   glDepthFunc(GL_LEQUAL);
   glEnable(GL_DEPTH_TEST);

   float a1, a2, a3, a4, a5, a6;
   a1 = a2 = a3 = a4 = a5 = a6 = 0.2;

   Map map(torus, std::chrono::high_resolution_clock::now());
   glm::vec3 camera_pos = {100, 100, 100};

   while (!glfwWindowShouldClose(window))
   {
      glfwPollEvents();

      // Get windows size
      int display_w, display_h;
      glfwGetFramebufferSize(window, &display_w, &display_h);

      // Set viewport to fill the whole window area
      glViewport(0, 0, display_w, display_h);

      // Fill background with solid color
      glClearColor(0.30f, 0.55f, 0.60f, 1.00f);
      glClear(GL_COLOR_BUFFER_BIT);

      // Gui start new frame
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      // GUI
      ImGui::Begin("Settings");
      ImGui::SliderInt("zoom sensitivity, %", &zoom_sensitivity, 0, 100);
      ImGui::SliderFloat("detail_coef", &detail_coef, 0, 5);
      ImGui::SliderInt("detail_repeat_count", &detail_repeat_count, 1, 100);
      ImGui::InputFloat("detail_dist", &detail_dist);
      ImGui::SliderInt("tex1_repeat_count", &tex1_repeat_count, 1, 100);
      ImGui::SliderInt("tex2_repeat_count", &tex2_repeat_count, 1, 100);
      ImGui::SliderInt("tex3_repeat_count", &tex3_repeat_count, 1, 100);
      ImGui::SliderFloat("spring_coef", &spring_coef, 0.05f, 1.f);
      ImGui::InputInt("enable", &enable);
      ImGui::End();

        
      map.buttons_callback();
      int d_time = map.move(std::chrono::high_resolution_clock::now());

     
      glm::mat4 projection = glm::perspective<float>(90, float(display_w) / display_h, 0.1, 100);
      auto model_torus = torus.get_model_matrix();


      auto pos = map.get_point_on_torus();
      auto dir = map.get_direction_on_torus();
   
      auto model_obj = obj.get_model_matrix();

      glm::vec3 bottom_center = glm::vec3(model_obj * glm::vec4(obj.get_bottom_center(), 1));
      glm::vec3 torus_point = glm::vec3(model_torus * glm::vec4(torus.get_vertex(pos[0], pos[1]), 1));
      glm::vec3 torus_normal = glm::vec3(model_torus * glm::vec4(torus.get_normal(pos[0], pos[1]), 1));

      auto to_normal = glm::orientation(torus_normal, glm::vec3(1, 0, 0));
      glm::vec3 new_y = normalize(glm::vec3(to_normal * glm::vec4(0, 1, 0, 0)));
      glm::vec3 new_x = normalize(glm::vec3(to_normal * glm::vec4(1, 0, 0, 0)));
      glm::vec3 new_z = normalize(glm::vec3(to_normal * glm::vec4(0, 0, 1, 0)));
      glm::vec3 torus_dir_x = normalize(map.get_torus_dir_x());
      glm::vec3 torus_dir_y = normalize(map.get_torus_dir_y());
      glm::vec3 torus_dir = normalize(map.get_torus_dir());

      auto alpha = acos(dot(new_y, torus_dir_x));
      auto beta = acos(dot(new_z, torus_dir_x));
      if (dot(new_z, torus_dir_y) > 0) {
          beta = -beta;
      }

      auto rot1 = glm::rotate(beta, new_x);
      auto rot2 = glm::rotate(map.get_angle() + 3.14f / 2.f, new_x);
      model_obj = glm::translate(torus_point - bottom_center) * rot2 * rot1 * to_normal * model_obj;


      auto model_camera = torus.get_translation_matrix(pos, obj) * 
                          map.get_rotation_matrix() * 
                          obj.get_model_matrix();

      glm::vec3 next_camera_pos = glm::vec3(torus.get_translation_matrix(pos, obj) * glm::vec4(0.7, dir.x / 2.f, dir.y / 2.f, 1));

      float distanse = glm::distance(camera_pos, next_camera_pos);
      glm::vec3 shift = glm::normalize(next_camera_pos - camera_pos) * distanse * (spring_coef / map.get_time_delta() * d_time);
      camera_pos = distanse < 0.07f ? camera_pos : camera_pos + shift;
      if (distanse > 5) { camera_pos = next_camera_pos; }
      
      auto view = glm::lookAt(
         camera_pos,
         glm::vec3(torus.get_translation_matrix(pos, obj) * glm::vec4(0, 0, 0, 1)),
         glm::vec3(model_camera * glm::vec4(-1, 0, 0, 0))
      );




      auto light_near_projection = glm::ortho(-2.f, 2.f, -2.f, 2.f, 0.001f, 2.f);
      auto light_near_view = glm::lookAt(
         glm::vec3(torus.get_translation_matrix(pos, obj) * glm::vec4(0, 0, 0, 1)) + glm::vec3(0, 0, 1),
         glm::vec3(torus.get_translation_matrix(pos, obj) * glm::vec4(0, 0, 0, 1)),
         glm::vec3(0, 1, 0)
      );
      auto light_object_view = glm::lookAt(
         glm::vec3(model_obj * glm::vec4(0.001, 0, 20.f, 1)),
         glm::vec3(model_obj * glm::vec4(0, 0, 20.f, 1)),
         glm::vec3(model_obj * glm::vec4(-1, 0, 0, 0))
      );

      glm::mat4 light_far_projection = glm::ortho(-15.f, 15.f, -15.f, 15.f, 0.001f, 15.f);
      glm::mat4 light_far_view = glm::lookAt(glm::vec3(0, 0, 10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
      glm::mat4 light_object_projection = glm::perspective(glm::radians(90.f), 1.f, 0.001f, 1.f);
      
      auto vp_near = light_near_projection * light_near_view;
      auto vp_far = light_far_projection * light_far_view;
      auto vp_object = light_object_projection * light_object_view;;

      auto torus_mvp = vp_near * model_torus;
      auto object_mvp = vp_near * model_obj;
      near_shadow_map.render(shadow_shader, torus, obj, torus_mvp, object_mvp);
      torus_mvp = vp_far * model_torus;
      object_mvp = vp_far * model_obj;
      far_shadow_map.render(shadow_shader, torus, obj, torus_mvp, object_mvp);
      torus_mvp = vp_object * model_torus;
      object_mvp = vp_object * model_obj;
      object_shadow_map.render(shadow_shader, torus, obj, torus_mvp, object_mvp);



     
      glViewport(0, 0, display_w, display_h);
      glClear(unsigned(GL_COLOR_BUFFER_BIT) | unsigned(GL_DEPTH_BUFFER_BIT));


      // отключаем тест глубины, чтобы все рисовалось поверх environment
      glDepthMask(GL_FALSE);

      // рисуем окружение

      env_shader.use();
      env_shader.set_uniform("projection", glm::value_ptr(projection));
      env_shader.set_uniform("view", glm::value_ptr(view));
      env_shader.set_uniform("environment", 1);

      env.render(env_shader, cubemap_texture);

      glDepthMask(GL_TRUE);


      // рисуем тор
      
      torus_shader.use();
      torus_shader.set_uniform("model", glm::value_ptr(model_torus));
      torus_shader.set_uniform("view", glm::value_ptr(view));
      torus_shader.set_uniform("projection", glm::value_ptr(projection));
      torus_shader.set_uniform("detail_coef", detail_coef);
      torus_shader.set_uniform("detail_repeat_count", detail_repeat_count);
      torus_shader.set_uniform("detail_dist", detail_dist);
      torus_shader.set_uniform("tex1_repeat_count", tex1_repeat_count);
      torus_shader.set_uniform("tex2_repeat_count", tex2_repeat_count);
      torus_shader.set_uniform("tex3_repeat_count", tex3_repeat_count);
      torus_shader.set_uniform("near_shadow_map", (int) near_shadow_map.get_id());
      torus_shader.set_uniform("far_shadow_map", (int) far_shadow_map.get_id());
      torus_shader.set_uniform("object_shadow_map", (int) object_shadow_map.get_id());
      torus_shader.set_uniform("mvp_near", glm::value_ptr(vp_near));
      torus_shader.set_uniform("mvp_far", glm::value_ptr(vp_far));
      torus_shader.set_uniform("mvp_object", glm::value_ptr(vp_object));
      torus_shader.set_uniform("enable", enable);

      torus.render(torus_shader);


      // рисуем объект

      obj_shader.use();
      obj_shader.set_uniform("model", glm::value_ptr(model_obj));
      obj_shader.set_uniform("view", glm::value_ptr(view));
      obj_shader.set_uniform("projection", glm::value_ptr(projection));
      obj_shader.set_uniform("obj_texture", 0);
      obj_shader.set_uniform("cubemap_texture", 1);
      obj_shader.set_uniform("near_shadow_map", (int) near_shadow_map.get_id());
      obj_shader.set_uniform("mvp_near", glm::value_ptr(vp_near));
   
      obj.render(obj_shader, obj_textute, cubemap_texture);


      glBindVertexArray(0);

      // Generate gui render commands
      ImGui::Render();

      // Execute gui render commands using OpenGL backend
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      // Swap the backbuffer with the frontbuffer that is used for screen display
      glfwSwapBuffers(window);

   }

   // Cleanup
   ImGui_ImplOpenGL3_Shutdown();
   ImGui_ImplGlfw_Shutdown();
   ImGui::DestroyContext();

   glfwDestroyWindow(window);
   glfwTerminate();

   return 0;
}
