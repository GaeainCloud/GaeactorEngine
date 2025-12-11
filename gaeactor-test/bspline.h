#pragma once

#include<iostream>
#include <cmath>
#include <ctime>
#include<vector>
constexpr auto deltaTIME = 50;

enum Type//B样条类型
{
    uniform,//均匀
    quniform//准均匀
};

class Point//点
{
public:
    double x;
    double y;
};


struct tagPoint {
    float x, y, z;
    tagPoint(float x = 0, float y = 0 ,float z=0):x(x),y(y),z(z){}
};
// 计算组合数
extern int binomial(int n, int i);
// 计算n次贝塞尔曲线上的点
extern tagPoint bezier_curve(const std::vector<tagPoint>& points, float t);


class Bspline //B样条曲线
{
public:
    Bspline(int _k, int _type, std::vector<Point> _p, float step);
    ~Bspline();
    void delay(int time); //延时函数，单位ms
    double BsplineBfunc(int i, int k, double uu);//计算每个u和每个i对应的B样条
    std::vector<Point> creatBspline();//计算整个的B样条

public:
    int k;//阶数
    int n;//控制点数-1
    int type; //B样条类型
    std::vector<double> u;//自变量
    double delta_u = 0.01;//自变量间隔
    double uBegin;
    double uEnd;
    std::vector<Point> p;//控制点
    std::vector<Point> pTrack;//轨迹点
};
