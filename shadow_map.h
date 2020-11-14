#pragma once
#include <GL/glew.h>


class Shadow_map {

    private:

    GLuint texture_id;
    GLuint buffer_id;
    int width = 1024;
    int height = 1024;

    public:

    Shadow_map() {
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,
                     GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
        glBindTexture(GL_TEXTURE_2D, 0);
        glGenFramebuffers(1, &buffer_id);
        glBindFramebuffer(GL_FRAMEBUFFER, buffer_id);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture_id, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    GLuint get_id() {
        return texture_id; 
    }

    template<class U, class V>
    void render(
        shader_t shadow_shader,
        U obj1,
        V obj2,
        glm::mat4 mvp1,
        glm::mat4 mvp2
    ) {
        glBindFramebuffer(GL_FRAMEBUFFER, buffer_id);
        glViewport(0, 0, width, height);
        glClear(GL_DEPTH_BUFFER_BIT);
        shadow_shader.use();
        shadow_shader.set_uniform("mvp", glm::value_ptr(mvp1));
        obj1.render();
        shadow_shader.set_uniform("mvp", glm::value_ptr(mvp2));
        obj2.render();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glActiveTexture(GL_TEXTURE0 + texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);
    }

};