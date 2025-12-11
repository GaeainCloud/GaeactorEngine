#version 330 core

in vec2 FragCoord; // 输入片段坐标
out vec4 FragColor; // 输出颜色

uniform vec2 center; // 渐变中心点
uniform vec3 color1; // 渐变起始颜色
uniform vec3 color2; // 渐变结束颜色
uniform float radius; // 渐变半径

void main()
{
    // 计算片段到中心点的距离
    float dist = distance(center, FragCoord);

    // 计算当前片段的角度（0到1之间的值，表示从起始到结束的百分比）
    float angle = atan(FragCoord.y - center.y, FragCoord.x - center.x) / (2.0 * 3.14159) + 0.5;

    // 根据角度计算颜色插值
    vec3 interpolatedColor = mix(color2, color1, angle);

    // 根据距离与半径的比例来混合颜色
    float gradient = smoothstep(0.0, radius, dist);

    // 输出最终颜色
    FragColor = vec4(interpolatedColor, gradient);
}
