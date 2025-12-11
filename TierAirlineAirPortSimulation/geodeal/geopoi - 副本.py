import geopandas as gpd
from shapely.geometry import LineString, Point, MultiPoint

from geopy.distance import geodesic
#from geographiclib.geodesic import Geodesic
import math
import json
import random
# def calculate_angle(line1, line2):
#     # 获取两个 LineString 的起点和终点坐标
#     p1, p2 = line1.coords
#     p3, p4 = line2.coords
    
#     # 计算两个 LineString 的向量表示
#     vector1 = (p2[0] - p1[0], p2[1] - p1[1])
#     vector2 = (p4[0] - p3[0], p4[1] - p3[1])
    
#     # 计算两个向量的点积
#     dot_product = vector1[0] * vector2[0] + vector1[1] * vector2[1]
    
#     # 计算两个向量的长度
#     length1 = math.sqrt(vector1[0] ** 2 + vector1[1] ** 2)
#     length2 = math.sqrt(vector2[0] ** 2 + vector2[1] ** 2)
    
#     # 计算夹角的余弦值
#     cos_angle = dot_product / (length1 * length2)
    
#     # 计算夹角（弧度制）
#     angle_rad = math.acos(cos_angle)
    
#     # 将弧度转换为角度
#     angle_deg = math.degrees(angle_rad)
    
#     return angle_deg
# 解析 JSON 字符串为 Python 字典
with open('Agent.json', 'r', encoding='utf-8') as file:
    # 解析 JSON 文件
    data = json.load(file)




pois = data['pois']

def getMinDistance(point):
    # 初始化最小距离和对应的点索引
    min_distance = float('inf')
    min_distance_point = ''
    for poi in pois:
        # 访问各个字段的值
        poi_keyword = poi['poiKeyword']
        poi_point = poi['poiPoint']

        point1 = (point.y, point.x)
        point2=(poi_point[1], poi_point[0])

        distance = geodesic(point1, point2).meters

        if distance < min_distance and distance < 50:
            min_distance = distance
            min_distance_point = poi_keyword
    return min_distance_point
    # 创建一个 Point 对象

    # # 遍历每个 LineString 几何对象
    # for line in gdf.geometry:
    #     # 计算 LineString 的最近点
    #     nearest_point = line.interpolate(line.project(point))
    #     print(nearest_point)

def calc_intersection_point(line1, line2):
    # 创建线段对象
    # # 计算两个 LineString 的夹角
    # angle = calculate_angle(line1, line2)
    # print("两个 LineString 的夹角（单位：度）:", angle)

    if line1.intersects(line2):
        intersection = line1.intersection(line2)
        return intersection
    else:
        return None

cc = 0
# 读取 GeoJSON 文件
gdf = gpd.read_file("taxiway.geojson")

poi_pois={}
midpoints = []

def appendpoi_pois(point):
    if point not in midpoints:
        midpoints.append(point)
        poikey = getMinDistance(point)
        if not poikey:
            return
        if poikey not in poi_pois:
            poi_pois[poikey] = []
        poi_pois[poikey].append(point)


# 遍历每个 LineString 几何对象
for line in gdf.geometry:
    cc = cc + 1
    print(f"{cc} / {gdf.geometry.size}")
    for line2 in gdf.geometry:
        intersectionsas = calc_intersection_point(line,line2)
        if intersectionsas is None:
            continue
        if isinstance(intersectionsas, Point):
            appendpoi_pois(intersectionsas)
        elif isinstance(intersectionsas, MultiPoint):
            for point in intersectionsas.geoms:
                appendpoi_pois(point)

    if line.length <= 100/100000:
        midpoint = line.interpolate(0.5, normalized=True)
        appendpoi_pois(midpoint)

'''
for line in gdf.geometry:
    if line.length <= 100/100000:
        midpoint = line.interpolate(0.5, normalized=True)
        if midpoint not in midpoints:
            midpoints.append(midpoint)
            poikey = getMinDistance(midpoint)
            if poikey not in poi_pois:
                poi_pois[poikey] = []
            poi_pois[poikey].append(midpoint)

            filename = "./result/"+poikey+"_point.geojson"
            gdf_point = gpd.GeoDataFrame(geometry=poi_pois[poikey])
            gdf_point['poiKeyword']=poikey
            gdf_point.to_file(filename, driver="GeoJSON")
'''

def getcolor():
    # 生成随机的 RGB 值
    r = random.randint(0, 255)
    g = random.randint(0, 255)
    b = random.randint(0, 255)

    # 将 RGB 值转换为十六进制格式
    color_code = '#{:02x}{:02x}{:02x}'.format(r, g, b)
    return color_code

multipointw=[]
gdfout_poiKeyword=[]
gdfout_color=[]

poi_pois_result=[]

cc = 0
for key, value in poi_pois.items():
    multipointw.append(MultiPoint(value))
    gdfout_poiKeyword.append(key)
    gdfout_color.append(getcolor())
    #filename = "./result/"+key+"_multipoint.geojson"
    gdf_multipoint = gpd.GeoDataFrame(geometry=[MultiPoint(value)])
    gdf_multipoint['poiKeyword']=key
    # gdf_multipoint.to_file(filename, driver='GeoJSON')

    # filename = "./result/"+key+"_point.geojson"
    #gdf_point = gpd.GeoDataFrame(geometry=value)
    #gdf_point['poiKeyword']=key
    # gdf_point.to_file(filename, driver="GeoJSON")
    print(key, value)
    cc = cc + 1
    pois_ret={}
    pois_ret['poiKey'] = 'poi_'+str(cc)
    pois_ret['poiKeyword'] = 'res_'+key

    poiGeoJSON={}
    features=[]
    feature={}
    feature['type']='Feature'
    feature['properties']={}
    feature['geometry']={}
    feature['geometry']['type']='MultiPoint'
    coordinates=[]
    for point in value:        
        coordinates.append([point.x, point.y])
    feature['geometry']['coordinates']=[coordinates]
    features.append(feature)

    poiGeoJSON['type']='FeatureCollection'
    poiGeoJSON['features']=features

    pois_ret['poiGeoJSON'] = poiGeoJSON
    pois_ret['poiName'] = 'res_'+key
    pois_ret['poiNameI18n'] = 'res_'+key
    pois_ret['poiVarDefs']=[]
    poiLabels=[]
    poiLabels.append('res_pt')
    pois_ret['poiLabels']=poiLabels
    pois_ret['poiDirection']=-1
    pois_ret['poiFrame']=0
    pois_ret['poiPoint']=[]
    poi_pois_result.append(pois_ret)

with open("pois.json", 'w') as f:
    json.dump(poi_pois_result, f, indent=4)

gdfout = gpd.GeoDataFrame(geometry=multipointw)
gdfout['poiKeyword']=gdfout_poiKeyword
gdfout['marker-color']=gdfout_color
# 保存为 GeoJSON 文件
gdfout.to_file("output2.geojson", driver="GeoJSON")
