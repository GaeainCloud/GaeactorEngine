#ifndef CLOUDNODE_H
#define CLOUDNODE_H
#include <osg/Matrixd>
#include <osg/Program>
#include <string>
class CloudNode
{
public:
    CloudNode(const std::string& nodename);
    ~CloudNode();
    void updateLightMatrix(const osg::Matrixd& lightMatrix);
private:
    void init();
private:


    osg::ref_ptr<osg::Program> pShaderProgram;
};
#endif // MAINWINDOW_H
