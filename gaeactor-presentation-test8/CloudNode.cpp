#pragma execution_character_set("utf-8")
#include "CloudNode.h"


CloudNode::CloudNode(const std::string &nodename)
{
}

CloudNode::~CloudNode()
{

}

void CloudNode::updateLightMatrix(const osg::Matrixd &lightMatrix)
{

    //                                        // 创建统一变量并设置值
    //                                        osg::ref_ptr<osg::Uniform> lineLengthUniform = new osg::Uniform("lineLength", params.lineLength);
    //                                        planetGeometry->getOrCreateStateSet()->addUniform(lineLengthUniform);
//    pShaderProgram->geto
}

void CloudNode::init()
{

}
