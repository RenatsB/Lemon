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
    m_transform.setScale(2.0f,2.0f,2.0f);
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
    /*shader->loadShader("DispColour",
                       "shaders/dispColour/DispColour_Vert.glsl",
                       "shaders/dispColour/DispColour_Frag.glsl");
    (*shader)["DispColour"]->use();*/
    shader->loadShader("DispColour",
                       "shaders/simplePhong/LemonPhong_Vert.glsl",
                       "shaders/simplePhong/LemonPhong_Frag.glsl");
    (*shader)["DispColour"]->use();
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
}

void LemonScene::paintGL()
{
    //m_transform.addRotation(ngl::Vec3(0.f,0.5f,0.f));
    glViewport(0,0,m_win.width,m_win.height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glPatchParameteri( GL_PATCH_VERTICES, 3 );
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
    }
    if(m_currentShader==PBR)
    {
        shader->setUniform("albedo",0.5f, 0.0f, 0.0f);
        shader->setUniform("ao",1.0f);
        shader->setUniform("roughness",0.02f);
        shader->setUniform("camPos",m_cam.getEye().toVec3());
        shader->setUniform("exposure",1.0f);
        for(size_t i=0; i<g_lightPositions.size(); ++i)
        {
          shader->setUniform(("lightPositions[" + std::to_string(i) + "]").c_str(),g_lightPositions[i]);
          shader->setUniform(("lightColors[" + std::to_string(i) + "]").c_str(),m_lightColors[i]);
        }
    }


    loadMatricesToShader();

    ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
    prim->draw("sp1");

    //ngl::Transformation temp = m_transform;
    //m_transform.reset();
    //m_transform.setPosition(ngl::Vec3(0,0,1));
    //m_transform.setRotation(ngl::Vec3(90,0,0));

    loadMatricesToShader();
    prim->draw("plane1");

    //m_transform.reset();
    //m_transform = temp;
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
    default:
      break;
  }
  update();
}

