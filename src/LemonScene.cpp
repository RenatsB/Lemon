#include "LemonScene.h"
#include <QGuiApplication>
#include <QMouseEvent>

#include <ngl/Camera.h>
#include <ngl/NGLInit.h>
#include <ngl/NGLStream.h>
#include <ngl/Random.h>
#include <ngl/ShaderLib.h>
#include <ngl/VAOPrimitives.h>

std::array<ngl::Vec3,4> g_lightPositions = {{
        ngl::Vec3(-7.0f,  4.0f, -7.0f),
        ngl::Vec3( 7.0f,  4.0f, -7.0f),
        ngl::Vec3(-7.0f,  4.0f, 7.0f),
        ngl::Vec3( 7.0f,  4.0f, 7.0f)
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
    m_transform.setScale(1.0f,1.0f,1.0f);
    // Set background colour
    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);

    glEnable( GL_DEPTH_TEST );
    #ifndef USINGIOS_
      glEnable( GL_MULTISAMPLE );
    #endif

    ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
    //generate primitives
    prim->createSphere("sp1", 0.8f, 68);
    prim->createSphere("sphere", 0.5f, 12);
    prim->createTrianglePlane("plane1", 1, 1, 24, 24, ngl::Vec3(0,1,0));

    // Create and compile shaders
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    //----------------------------------------------------
    //------[       PHONG       ]-------------------------
    //----------------------------------------------------
    shader->loadShader("LemonPhong",
                       "shaders/simplePhong/LemonPhong_Vert.glsl",
                       "shaders/simplePhong/LemonPhong_Frag.glsl");
    (*shader)["LemonPhong"]->use();
    //----------------------------------------------------
    //------[        PBR        ]-------------------------
    //----------------------------------------------------
    shader->loadShader("LemonPBR",
                       "shaders/lemonPBR/LemonPBR_Vert.glsl",
                       "shaders/lemonPBR/LemonPBR_Frag.glsl");
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
    shader->loadShaderSource( vertexShader, "shaders/tessellationTest/TesTest_Vert.glsl" );
    shader->loadShaderSource( tesevalShader, "shaders/tessellationTest/TesTest_Tese.glsl" );
    shader->loadShaderSource( tesctrlShader, "shaders/tessellationTest/TesTest_Tesc.glsl" );
    shader->loadShaderSource( geometryShader, "shaders/tessellationTest/TesTest_Geo.glsl" );
    shader->loadShaderSource( fragShader, "shaders/tessellationTest/TesTest_Frag.glsl" );
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
                       "shaders/noiseShader/Noise_Vert.glsl",
                       "shaders/noiseShader/Noise_Frag.glsl");
    (*shader)["NoiseDisplay"]->use();
    //----------------------------------------------------
    //------[     DispColour    ]-------------------------
    //----------------------------------------------------
    shader->loadShader("DispColour",
                       "shaders/dispColour/DispColour_Vert.glsl",
                       "shaders/dispColour/DispColour_Frag.glsl");
    (*shader)["DispColour"]->use();
    //----------------------------------------------------
    //------[    PhongMapped    ]-------------------------
    //----------------------------------------------------
    shader->loadShader("PhongMapped",
                       "shaders/lemonPhongMapped/LemonPhongMapped_Vert.glsl",
                       "shaders/lemonPhongMapped/LemonPhongMapped_Frag.glsl");
    (*shader)["PhongMapped"]->use();
    //----------------------------------------------------
    //------[     PBRMapped     ]-------------------------
    //----------------------------------------------------
    shader->loadShader("PBRMapped",
                       "shaders/lemonPBRMapped/LemonPBRMapped_Vert.glsl",
                       "shaders/lemonPBRMapped/LemonPBRMapped_Frag.glsl");
    (*shader)["PBRMapped"]->use();
    //----------------------------------------------------
    //set up cam
    ngl::Vec3 from( 0, 5, 13 );
    ngl::Vec3 to( 0, 0, 0 );
    ngl::Vec3 up( 0, 1, 0 );
    //load to cam
    m_cam.set( from, to, up );
    // set the shape using FOV 45 Aspect Ratio based on Width and Height
    // The final two are near and far clipping planes of 0.5 and 10
    m_cam.setShape( 45.0f, 720.0f / 576.0f, 0.05f, 350.0f );

    initTextureWriter(0, 4);

    glBindFramebuffer( GL_FRAMEBUFFER,  4 );
    generateNoiseTexture(0);
    glBindFramebuffer( GL_FRAMEBUFFER,  0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, 0 );

    glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glGenerateMipmap( GL_TEXTURE_2D );
}

void LemonScene::paintGL()
{
    glViewport(0,0,m_win.width,m_win.height);
    glClearColor( 0.5, 0.5, 0.5, 1.0f );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use our shader for this draw
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();

    // Rotation based on the mouse position for our global transform
    ngl::Mat4 rotX;
    ngl::Mat4 rotY;
    // create the rotation matrices
    rotX.rotateX( m_win.spinXFace );
    rotY.rotateY( m_win.spinYFace );
    // multiply the rotations
    m_mouseGlobalTX = rotX * rotY;
    // add the translations
    m_mouseGlobalTX.m_m[ 3 ][ 0 ] = m_modelPos.m_x;
    m_mouseGlobalTX.m_m[ 3 ][ 1 ] = m_modelPos.m_y;
    m_mouseGlobalTX.m_m[ 3 ][ 2 ] = m_modelPos.m_z;

    switch(m_currentShader)
    {
      case(PBR):{(*shader)["LemonPBR"]->use();break;}
      case(TessTest):{(*shader)["TessTest"]->use();break;}
      case(NoiseDisplay):{(*shader)["NoiseDisplay"]->use();break;}
      case(DispColour):{(*shader)["DispColour"]->use();break;}
      case(Phong):{(*shader)["LemonPhong"]->use();break;}
      case(PhongMapped):{(*shader)["PhongMapped"]->use();shader->setUniform("NormalTexture",0);break;}
      case(PBRMapped):{(*shader)["PBRMapped"]->use();shader->setUniform("NormalTexture",0);break;}
    }
    shader->setUniform("albedo",1.0f, 1.0f, 1.0f);
    shader->setUniform("ao",1.0f);
    shader->setUniform("metallic",0.46f);
    shader->setUniform("roughness",0.15f);
    shader->setUniform("camPos",m_cam.getEye().toVec3());
    shader->setUniform("exposure",1.0f);
    if(m_currentShader==PBR)
    {

        for(size_t i=0; i<g_lightPositions.size(); ++i)
        {
          shader->setUniform(("lightPositions[" + std::to_string(i) + "]").c_str(),g_lightPositions[i]);
          shader->setUniform(("lightColors[" + std::to_string(i) + "]").c_str(),m_lightColors[i]);
        }
    }

    loadMatricesToShader();

    ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
    prim->draw("sp1");

    (*shader)["NoiseDisplay"]->use();

    m_transform.reset();
    //m_transform = temp;
    for(auto p : g_lightPositions)
    {
      m_transform.setPosition(p);
      loadMatricesToShader();
      prim->draw("sphere");
      m_transform.reset();
    }
    m_transform.reset();

}

void LemonScene::loadMatricesToShader()
{
  ngl::ShaderLib* shader = ngl::ShaderLib::instance();

  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;
  M            = m_mouseGlobalTX * m_transform.getMatrix() ;
  MV           = m_cam.getViewMatrix() * M;
  MVP          = m_cam.getVPMatrix() * M;

  normalMatrix = MV;
  normalMatrix.inverse().transpose();
  shader->setUniform( "MVP", MVP );
  shader->setUniform( "N", normalMatrix );
  shader->setUniform( "M", M );
  shader->setUniform( "MV", MV );
}

void LemonScene::keyPressEvent( QKeyEvent* _event )
{
  // that method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch ( _event->key() )
  {
    // escape key to quit
    case Qt::Key_Escape:
      QGuiApplication::exit( EXIT_SUCCESS );
      break;
// turn on wirframe rendering
#ifndef USINGIOS_
    case Qt::Key_W:
      glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
      break;
    // turn off wire frame
    case Qt::Key_S:
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
      break;
#endif
    // show full screen
    case Qt::Key_F:
      showFullScreen();
      break;
    // show windowed
    case Qt::Key_N:
      showNormal();
      break;
    case Qt::Key_Space :
      m_win.spinXFace=0;
      m_win.spinYFace=0;
      m_modelPos.set(ngl::Vec3::zero());
    break;
    case Qt::Key_0 :
      m_currentShader = Phong;
    break;
    case Qt::Key_1 :
      m_currentShader = PBR;
    break;
    case Qt::Key_2 :
      m_currentShader = TessTest;
    break;
    case Qt::Key_3 :
      m_currentShader = NoiseDisplay;
    break;
    case Qt::Key_4 :
      m_currentShader = DispColour;
    break;
    case Qt::Key_5 :
      m_currentShader = PhongMapped;
    break;
    case Qt::Key_6 :
      m_currentShader = PBRMapped;
    break;
    default:
      break;
  }
  update();
}

void LemonScene::initTextureWriter(GLuint _texID, GLuint _FBO)
{
  //call this once for every FBO ID

  glGenFramebuffers(1, &_FBO);
  glBindFramebuffer(GL_FRAMEBUFFER, _FBO);

  glGenTextures(1, &_texID);
  glBindTexture(GL_TEXTURE_2D, _texID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_win.width, m_win.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texID, 0);
}

void LemonScene::generateNoiseTexture(GLuint _texID)
{
      //call this only after initTextureWriter

      glViewport( 0, 0, m_win.width, m_win.width );
      glClearColor( 1, 1, 1, 1.0f );
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

      ngl::ShaderLib *shader=ngl::ShaderLib::instance();
      ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();

      // Rotation based on the mouse position for our global transform
      ngl::Mat4 rotX;
      ngl::Mat4 rotY;
      // create the rotation matrices
      rotX.rotateX( m_win.spinXFace );
      rotY.rotateY( m_win.spinYFace );
      // multiply the rotations
      m_mouseGlobalTX = rotX * rotY;
      // add the translations
      m_mouseGlobalTX.m_m[ 3 ][ 0 ] = m_modelPos.m_x;
      m_mouseGlobalTX.m_m[ 3 ][ 1 ] = m_modelPos.m_y;
      m_mouseGlobalTX.m_m[ 3 ][ 2 ] = m_modelPos.m_z;

      glActiveTexture( GL_TEXTURE0 );
      glBindTexture( GL_TEXTURE_2D, _texID);

      glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
      glGenerateMipmap( GL_TEXTURE_2D );

      (*shader)["NoiseDisplay"]->use();

      //make scene here
      m_transform.reset();
      m_transform.setRotation(ngl::Vec3(90,0,0));
      m_transform.setScale(ngl::Vec3(2.f,2.f,2.f));

      loadMatricesToShader();

      prim->draw("plane1");
      m_transform.reset();
}

//------------------------------------------------------------------------------------------------------------------------------
