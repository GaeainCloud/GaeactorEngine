#include "bspline.h"



Bspline::Bspline(int _k, int _type, std::vector<Point> _p, float step)
{
    k = _k;
    n = _p.size() - 1;
    if (k > n + 1 || _p.empty())//k必需<=n+1， 不能一个控制点都没有
    {
        system("pause");
        exit(0);
    }

    type = _type;
    p = _p;

    double u_tmp = 0.0;
    u.push_back(u_tmp);//先把0.0存入

    if (type == uniform)//均匀
    {
        double dis_u = 1.0 / (k + n);
        for (int i = 1; i < n + k + 1; i++) //n + k + 1个分段
        {
            u_tmp += dis_u;
            u.push_back(u_tmp);
        }
    }
    else if (type == quniform)//准均匀
    {
        int j = 3;//重复度
        double dis_u = 1.0 / (k + n - (j - 1) * 2);
        for (int i = 1; i < j; i++)
        {
            u.push_back(u_tmp);
        }
        for (int i = j; i < n + k - j + 2; i++)
        {
            u_tmp += dis_u;
            u.push_back(u_tmp);
        }
        for (int i = n + k - j + 2; i < n + k + 1; i++)//n + k + 1个分段
        {
            u.push_back(u_tmp);
        }
    }

    delta_u = step;

    uBegin = u[k - 1];
    uEnd = u[n + 1];//计算u的区间


}

Bspline::~Bspline()
{
    p.clear();
    u.clear();
    pTrack.clear();
}

void Bspline::delay(int time) //延时函数，单位ms
{
    clock_t  now = clock();
    while (clock() - now < time)
    {

    }
}

double Bspline::BsplineBfunc(int i, int k, double uu)//计算每个u和每个i对应的B样条
{
    //cout << "****************i= " << i << endl;
    /*if (i == n + 1)
    {
        return 0.0;//防止越界
    }*/

    double Bfunc = 0.0;

    if (k == 1)//递归退出的条件
    {
        if (u[i] <= uu && uu < u[i + 1])
        {
            Bfunc = 1.0;
        }
        else
        {
            Bfunc = 0.0;
        }
    }
    else if (k >= 2)
    {
        double A = 0.0;
        double B = 0.0;

        if (u[i + k - 1] - u[i] == 0.0)
        {
            //cout << "A = 0.0; u[i+k-1]= " << u[i + k - 1] << ", u[i]= " << u[i] << endl;
            A = 0.0;//约定分母为0时，整个除式为0
        }
        else
        {
            A = (uu - u[i]) / (u[i + k - 1] - u[i]);

            /*if (A <= 0.0)
            {
                cout << "A < 0.0; A= " << A << ", uu= " << uu << ", u[i]= " << u[i] << ", u[i + k - 1]= " << u[i + k - 1] << ", i= " << i << ", k= " << k << endl;
            }*/
        }

        if (u[i + k] - u[i + 1] == 0.0)
        {
            //cout << "B = 0.0; u[i + k]= " << u[i + k] << ", u[i + 1] " << u[i + 1] << endl;
            B = 0.0;//约定分母为0时，整个除式为0
        }
        else
        {
            B = (u[i + k] - uu) / (u[i + k] - u[i + 1]);

            /*if (B <= 0.0)
            {
                cout << "B < 0.0; B= " << B << ", uu= " << uu << ", u[i]= " << u[i] << ", u[i + k]= " << u[i + k] << ", u[i + 1]= " << u[i + 1] << ", i= " << i << ", k= " << k << endl;
            }*/
        }

        Bfunc = A * BsplineBfunc(i, k - 1, uu) + B * BsplineBfunc(i + 1, k - 1, uu);//递归
    }

    //cout << "Bfunc= " << Bfunc << endl;
    return Bfunc;
}

std::vector<Point> Bspline::creatBspline()//计算整个的B样条
{
    for (double uu = uBegin; uu <= uEnd; uu += delta_u)//u的循环放外层，对应每个u，去遍历所有控制点
    {
        Point Pu = { 0.0, 0.0 };//每轮循环初始化
        for (int i = 0; i < n + 1; i++)//i从0到n，每个控制点
        {
            double xtmp = p[i].x;
            double ytmp = p[i].y;
            double BfuncTmp = BsplineBfunc(i, k, uu);
            Pu.x += xtmp * BfuncTmp;
            Pu.y += ytmp * BfuncTmp;//累加
        }
        pTrack.push_back(Pu);//轨迹点
    }
    return pTrack;


}

int binomial(int n, int i)
{
    int res = 1;
    for (int j = 1; j <= i; ++j) {
        res *= (n - j + 1) / (double)j; //(double)十分关键，不然j=i=n时，j为分数=0；
    }
    return res;
}

tagPoint bezier_curve(const std::vector<tagPoint> &points, float t)
{
    int n = points.size() - 1;
    tagPoint res;
    for (int i = 0; i <= n; ++i) {
        //cout << "p:" << points[i].x << "," << points[i].y << endl;
        float b =  binomial(n, i)* pow(t, i) * pow(1 - t, n - i);
        //        cout << "bino=" << binomial(n, i) << endl;
        binomial(n, i);
        res.x = res.x + points[i].x * b;
        res.y = res.y + points[i].y * b;
        res.z = res.z + points[i].z * b;
    }
    return res;
}
