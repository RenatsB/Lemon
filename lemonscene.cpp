#include "lemonscene.h"

#include <glm/gtc/type_ptr.hpp>
#include <ngl/Obj.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <ngl/Image.h>

LemonScene::LemonScene() : Scene() {}


void LemonScene::initGL() noexcept {
    // Fire up the NGL machinary (not doing this will make it crash)
    ngl::NGLInit::instance();

    // Set background colour
    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);

    // enable depth testing for drawing
    glEnable(GL_DEPTH_TEST);

    // enable multisampling for smoother drawing
    glEnable(GL_MULTISAMPLE);

    ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
    prim->createSphere("sp1", 0.8f, 64);
    prim->createTrianglePlane("plane1", 3, 3, 24, 24, ngl::Vec3(0,1,0));

    // Create and compile the vertex and fragment shader
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();

    shader->loadShader("TexGenShader",
                       "shaders/texgen_vert.glsl",
                       "shaders/texgen_frag.glsl");
    (*shader)["TexGenShader"]->use();

    GLuint pidtg = shader->getProgramID("TexGenShader");

    // Our MVP matrices
    glm::mat4 M = glm::mat4(1.0f);
    glm::mat4 MVP, MV;
    glm::mat3 N;

    // Note the matrix multiplication order as we are in COLUMN MAJOR storage
    MV = m_V * M;
    N = glm::inverse(glm::mat3(MV));
    MVP = m_P * MV;

    // Set this MVP on the GPU
    glUniformMatrix4fv(glGetUniformLocation(pidtg, "MVP"), //location of uniform
                       1, // how many matrices to transfer
                       false, // whether to transpose matrix
                       glm::value_ptr(MVP)); // a raw pointer to the data

    shader->loadShader("TestShader",
                       "shaders/test_vert.glsl",
                       "shaders/test_frag.glsl");
    (*shader)["TestShader"]->use();
}

void LemonScene::paintGL() noexcept {
    // Clear the screen (fill with our glClearColor)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up the viewport
    glViewport(0,0,m_width,m_height);

    // Use our shader for this draw
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    (*shader)["TestShader"]->use();
    GLuint pid = shader->getProgramID("TestShader");

    // Our MVP matrices
    glm::mat4 M = glm::mat4(1.0f);
    glm::mat4 MVP, MV;
    glm::mat3 N;

    // Note the matrix multiplication order as we are in COLUMN MAJOR storage
    MV = m_V * M;
    N = glm::inverse(glm::mat3(MV));
    MVP = m_P * MV;

    // Set this MVP on the GPU
    glUniformMatrix4fv(glGetUniformLocation(pid, "MVP"), //location of uniform
                       1, // how many matrices to transfer
                       false, // whether to transpose matrix
                       glm::value_ptr(MVP)); // a raw pointer to the data
    glUniformMatrix4fv(glGetUniformLocation(pid, "MV"), //location of uniform
                       1, // how many matrices to transfer
                       false, // whether to transpose matrix
                       glm::value_ptr(MV)); // a raw pointer to the data
    glUniformMatrix3fv(glGetUniformLocation(pid, "N"), //location of uniform
                       1, // how many matrices to transfer
                       true, // whether to transpose matrix
                       glm::value_ptr(N)); // a raw pointer to the data
    glUniformMatrix3fv(glGetUniformLocation(pid, "P"), //location of uniform
                       1, // how many matrices to transfer
                       true, // whether to transpose matrix
                       glm::value_ptr(m_P)); // a raw pointer to the data
    ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
    prim->draw("sp1");
    //prim->draw("teapot");

    /*(*shader)["TexGenShader"]->use();
    GLuint pidp = shader->getProgramID("TexGenShader");
    glUniformMatrix4fv(glGetUniformLocation(pidp, "MVP"), //location of uniform
                       1, // how many matrices to transfer
                       false, // whether to transpose matrix
                       glm::value_ptr(MVP)); // a raw pointer to the data
    glUniformMatrix4fv(glGetUniformLocation(pidp, "MV"), //location of uniform
                       1, // how many matrices to transfer
                       false, // whether to transpose matrix
                       glm::value_ptr(MV)); // a raw pointer to the data
    glUniformMatrix3fv(glGetUniformLocation(pidp, "N"), //location of uniform
                       1, // how many matrices to transfer
                       true, // whether to transpose matrix
                       glm::value_ptr(N)); // a raw pointer to the data
    glUniformMatrix3fv(glGetUniformLocation(pidp, "P"), //location of uniform
                       1, // how many matrices to transfer
                       true, // whether to transpose matrix
                       glm::value_ptr(m_P)); // a raw pointer to the data
    prim->draw("plane1");*/
}
