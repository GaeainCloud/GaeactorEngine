#ifndef TEXTUREIMAGE2D_H
#define TEXTUREIMAGE2D_H

#include "base_define.h"
#include <osg/Texture2D>

class TextureImage2D
{
public:
    TextureImage2D();
    TextureImage2D(const std::string& path, const osg::Texture::WrapMode& wrapParam = osg::Texture::REPEAT, const osg::Texture::FilterMode& minFilter = osg::Texture::LINEAR_MIPMAP_LINEAR, const osg::Texture::FilterMode& magFilter = osg::Texture::LINEAR_MIPMAP_LINEAR);
    ~TextureImage2D();

    osg::ref_ptr<osg::Texture2D>  getTexture() const;
    UINT32 getWidth() const;
    UINT32 getHeight() const;
private:

    void LoadTextureFromFile(const std::string& path, const osg::Texture::WrapMode& wrapParam, const osg::Texture::FilterMode& minFilter, const osg::Texture::FilterMode& magFilter);
private:
    UINT32 m_width = 0;
    UINT32 m_height = 0;
    osg::ref_ptr<osg::Texture2D> m_pTexture;
};
#endif // MAINWINDOW_H
