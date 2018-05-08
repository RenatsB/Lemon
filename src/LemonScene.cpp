#include "LemonScene.h"

#include <glm/gtc/type_ptr.hpp>
#include <ngl/Obj.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <ngl/Image.h>

//#define TESSELLATION_ON
#define NOISETEST_ON

std::array<ngl::Vec3,4> g_lightPositions = {{
        ngl::Vec3(-10.0f,  4.0f, -10.0f),
        ngl::Vec3( 10.0f,  4.0f, -10.0f),
        ngl::Vec3(-10.0f,  4.0f, 10.0f),
        ngl::Vec3( 10.0f,  4.0f, 10.0f)
}};



LemonScene::LemonScene()
{
  setTitle( "Lemon PBR" );
}

LemonScene::~LemonScene()
{
  std::cout << "Shutting down NGL, removing VAO's and Shaders\n";
}

void LemonScene::resizeGL( int _w, int _h )
{
  m_cam.setShape( 45.0f, static_cast<float>( _w ) / _h, 0.05f, 350.0f );
  m_win.width  = static_cast<int>( _w * devicePixelRatio() );
  m_win.height = static_cast<int>( _h * devicePixelRatio() );
}

void LemonScene::initializeGL()
{
    ngl::NGLInit::instance();

    m_transform.reset();
    m_transform.setScale(0.2f,0.2f,0.2f);
    // Set background colour
    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);

    glEnable( GL_DEPTH_TEST );
    #ifndef USINGIOS_
      glEnable( GL_MULTISAMPLE );
    #endif

    ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
    //generate primitives
    prim->createSphere("sp1", 0.8f, 68);
    prim->createTrianglePlane("plane1", 1, 1, 24, 24, ngl::Vec3(0,1,0));

    // Create and compile shaders
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    //----------------------------------------------------
    //------[       PHONG       ]-------------------------
    //----------------------------------------------------
    shader->loadShader("LemonPhong",
                       "shaders/LemonPhong_Vert.glsl",
                       "shaders/LemonPhong_Frag.glsl");
    (*shader)["LemonPhong"]->use();
    //----------------------------------------------------
    //------[        PBR        ]-------------------------
    //----------------------------------------------------
    shader->loadShader("LemonPBR",
                       "shaders/LemonPBR_Vert.glsl",
                       "shaders/LemonPBR_Frag.glsl");
    (*shader)["LemonPBR"]->use();
    //----------------------------------------------------
    //------[     TESSTEST      ]-------------------------
    //----------------------------------------------------
    constexpr auto shaderProgram = "TessTest";
    constexpr auto vertexShader  = "TessShaderVertex";
    constexpr auto fragShader    = "TessShaderFragment";
    constexpr auto tesctrlShader    = "TessShaderTesCtrl";
    constexpr auto tesevalShader    = "TessShaderTesEval";
    constexpr auto geometryShader    = "TessShaderGeometry";
    // create the shader program
    shader->createShaderProgram( shaderProgram );
    // now we are going to create empty shaders for Frag and Vert
    shader->attachShader( vertexShader, ngl::ShaderType::VERTEX );
    shader->attachShader( tesevalShader, ngl::ShaderType::TESSEVAL );
    shader->attachShader( tesctrlShader, ngl::ShaderType::TESSCONTROL );
    shader->attachShader( geometryShader, ngl::ShaderType::GEOMETRY );
    shader->attachShader( fragShader, ngl::ShaderType::FRAGMENT );
    // attach the source
    shader->loadShaderSource( vertexShader, "shaders/tessellationTest/test_vert.glsl" );
    shader->loadShaderSource( tesevalShader, "shaders/tessellationTest/test_tes.glsl" );
    shader->loadShaderSource( tesctrlShader, "shaders/tessellationTest/test_tcs.glsl" );
    shader->loadShaderSource( geometryShader, "shaders/tessellationTest/test_geo.glsl" );
    shader->loadShaderSource( fragShader, "shaders/tessellationTest/test_frag.glsl" );
    // compile the shaders
    shader->compileShader( vertexShader );
    shader->compileShader( tesevalShader );
    shader->compileShader( tesctrlShader );
    shader->compileShader( geometryShader );
    shader->compileShader( fragShader );
    // add them to the program
    shader->attachShaderToProgram( shaderProgram, vertexShader );
    shader->attachShaderToProgram( shaderProgram, tesevalShader );
    shader->attachShaderToProgram( shaderProgram, tesctrlShader );
    shader->attachShaderToProgram( shaderProgram, geometryShader );
    shader->attachShaderToProgram( shaderProgram, fragShader );
    //finish
    shader->linkProgramObject( shaderProgram );
    ( *shader )[ shaderProgram ]->use();
    //----------------------------------------------------
    //------[    NoiseDisplay   ]-------------------------
    //----------------------------------------------------
    shader->loadShader("NoiseDisplay",
                       "shaders/noiseShader/texgen_vert.glsl",
                       "shaders/noiseShader/texgen_frag.glsl");
    (*shader)["NoiseDisplay"]->use();
    //----------------------------------------------------
    //------[     DispColour    ]-------------------------
    //----------------------------------------------------
    /*shader->loadShader("TessTest",
                       "shaders/texgen_vert.glsl",
                       "shaders/texgen_frag.glsl");
    (*shader)["TexGenShader"]->use();*/

    //set up cam
    ngl::Vec3 from( 0, 5, 13 );
    ngl::Vec3 to( 0, 0, 0 );
    ngl::Vec3 up( 0, 1, 0 );
    //load to cam
    m_cam.set( from, to, up );
    // set the shape using FOV 45 Aspect Ratio based on Width and Height
    // The final two are near and far clipping planes of 0.5 and 10
    m_cam.setShape( 45.0f, 720.0f / 576.0f, 0.05f, 350.0f );
}

void LemonScene::paintGL() noexcept {
    // Clear the screen (fill with our glClearColor)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    m_transform.addRotation(ngl::Vec3(0.f,0.5f,0.f));
    // Set up the viewport
    glViewport(0,0,m_width,m_height);
    //glPatchParameteri( GL_PATCH_VERTICES, 3 );
    // Use our shader for this draw
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    GLuint pid;
    #ifdef TESSELLATION_ON
    {
      (*shader)["TestShader"]->use();
      pid = shader->getProgramID("TestShader");
    }
    #else
    {
      (*shader)["LemonShader"]->use();
      pid = shader->getProgramID("LemonShader");
    }
    #endif

    loadMatricesToShader(pid);

    ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
    prim->draw("sp1");

    #ifdef NOISETEST_ON
    {
      ngl::Transformation temp = m_transform;
      m_transform.reset();
      m_transform.setPosition(ngl::Vec3(0,0,1));

      (*shader)["TexGenShader"]->use();
      GLuint pidp = shader->getProgramID("TexGenShader");
      loadMatricesToShader(pidp);
      prim->draw("plane1");

      m_transform.reset();
      m_transform = temp;
    }
    #endif
}

void LemonScene::loadMatricesToShader(GLuint _pid)
{
  glm::mat4 MV;
  glm::mat4 MVP;
  glm::mat3 N;
  glm::mat4 M;
  ngl::Mat4 mngl = m_transform.getMatrix();
  M[0][0]  = (GLfloat)mngl.m_openGL[0];
  M[0][1]  = (GLfloat)mngl.m_openGL[1];
  M[0][2]  = (GLfloat)mngl.m_openGL[2];
  M[0][3]  = (GLfloat)mngl.m_openGL[3];
  M[1][0]  = (GLfloat)mngl.m_openGL[4];
  M[1][1]  = (GLfloat)mngl.m_openGL[5];
  M[1][2]  = (GLfloat)mngl.m_openGL[6];
  M[1][3]  = (GLfloat)mngl.m_openGL[7];
  M[2][0]  = (GLfloat)mngl.m_openGL[8];
  M[2][1]  = (GLfloat)mngl.m_openGL[9];
  M[2][2]  = (GLfloat)mngl.m_openGL[10];
  M[2][3]  = (GLfloat)mngl.m_openGL[11];
  M[3][0]  = (GLfloat)mngl.m_openGL[12];
  M[3][1]  = (GLfloat)mngl.m_openGL[13];
  M[3][2]  = (GLfloat)mngl.m_openGL[14];
  M[3][3]  = (GLfloat)mngl.m_openGL[15];
  MV  = m_V * M;
  MVP = m_P * MV;

  N = glm::inverse(MV);

  glUniformMatrix4fv(glGetUniformLocation(_pid, "MVP"), //location of uniform
                     1, // how many matrices to transfer
                     false, // whether to transpose matrix
                     glm::value_ptr(MVP)); // a raw pointer to the data
  glUniformMatrix4fv(glGetUniformLocation(_pid, "MV"), //location of uniform
                     1, // how many matrices to transfer
                     false, // whether to transpose matrix
                     glm::value_ptr(MV)); // a raw pointer to the data
  glUniformMatrix3fv(glGetUniformLocation(_pid, "N"), //location of uniform
                     1, // how many matrices to transfer
                     true, // whether to transpose matrix
                     glm::value_ptr(N)); // a raw pointer to the data
  glUniformMatrix3fv(glGetUniformLocation(_pid, "P"), //location of uniform
                     1, // how many matrices to transfer
                     true, // whether to transpose matrix
                     glm::value_ptr(m_P)); // a raw pointer to the data
}
