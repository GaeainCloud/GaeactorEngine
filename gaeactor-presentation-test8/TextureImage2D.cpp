#pragma execution_character_set("utf-8")
#include "TextureImage2D.h"
#include <osgDB/readFile>
TextureImage2D::TextureImage2D()
{

}

TextureImage2D::TextureImage2D(const std::string& path,
                               const osg::Texture::WrapMode& wrapParam/* = osg::Texture::REPEAT*/,
                               const osg::Texture::FilterMode& minFilter/* = osg::Texture::LINEAR_MIPMAP_LINEAR*/,
                               const osg::Texture::FilterMode& magFilter/* = osg::Texture::LINEAR_MIPMAP_LINEAR*/)
{
    LoadTextureFromFile(path, wrapParam, minFilter, magFilter);
}

TextureImage2D::~TextureImage2D()
{

}

osg::ref_ptr<osg::Texture2D>  TextureImage2D::getTexture() const
{
    return m_pTexture;
}

UINT32 TextureImage2D::getWidth() const
{
    return m_width;
}

UINT32 TextureImage2D::getHeight() const
{
    return m_height;
}

void TextureImage2D::LoadTextureFromFile(const std::string& path, const osg::Texture::WrapMode& wrapParam, const osg::Texture::FilterMode& minFilter, const osg::Texture::FilterMode& magFilter)
{
    osg::ref_ptr<osg::Image> rpImage = osgDB::readImageFile(path);
    m_pTexture = new osg::Texture2D();

    m_pTexture->setImage(rpImage);
    m_pTexture->setWrap(osg::Texture::WRAP_S, wrapParam);//设置S方向的环绕模式
    m_pTexture->setWrap(osg::Texture::WRAP_T, wrapParam);//设置R方向的环绕模式
    m_pTexture->setFilter(osg::Texture::MIN_FILTER, minFilter);//设置R方向的环绕模式
    m_pTexture->setFilter(osg::Texture::MAG_FILTER, magFilter);//设置R方向的环绕模式

    m_pTexture->setMaxAnisotropy(16);
}
