#ifndef LEMONSCENE_H
#define LEMONSCENE_H

#include <ngl/Obj.h>
#include "scene.h"

class LemonScene : public Scene
{
public:
    LemonScene();

    /// Called when the scene needs to be painted
    void paintGL() noexcept;

    /// Called when the scene is to be initialised
    void initGL() noexcept;

private:
    GLuint m_colourTex, m_normalTex;

    /// Initialise a texture
    //void initTexture(const GLuint& /*texUnit*/, GLuint &/*texId*/, const char */*filename*/);

    //void initTextureWriter(GLuint _texID, GLuint _FBO, GLuint _RBO);
    //void generateTexture(GLuint _texID, GLuint _);
    //void cleanupGen(GLuint _FBO, GLuint _tex);
};

#endif // LEMONSCENE_H
