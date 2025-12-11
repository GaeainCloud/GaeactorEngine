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
with open('Untitled-2.json', 'r', encoding='utf-8') as file:
    # 解析 JSON 文件
    jsondata = json.load(file)




data = jsondata['features']
# 分类数据
poi_data = {}
poi_data_color = {}
for feature in data:
    poi_keyword = feature["properties"]["poiKeyword"]
    poi_color = feature["properties"]["marker-color"]
    if poi_keyword not in poi_data:
        poi_data[poi_keyword] = []
    poi_data[poi_keyword].append(feature["geometry"]["coordinates"])

    if poi_keyword not in poi_data_color:
        poi_data_color[poi_keyword] = ''
    poi_data_color[poi_keyword] = poi_color


# 创建 MultiPoint
multi_points = {}
for poi_keyword, coordinates in poi_data.items():
    multi_points[poi_keyword] = MultiPoint(coordinates)

# 将 MultiPoint 转换为 GeoJSON 字符串并打印
for poi_keyword, multi_point in multi_points.items():
    print(f"poiKeyword: {poi_keyword} -color：{poi_data_color[poi_keyword]}    - MultiPoint: {multi_point}")


multipointw=[]
gdfout_poiKeyword=[]
gdfout_color=[]

poi_pois_result=[]
cc = 0
for poi_keyword, coordinates in poi_data.items():
    multipointw.append(MultiPoint(coordinates))
    gdfout_poiKeyword.append(poi_keyword)
    gdfout_color.append(poi_data_color[poi_keyword])
    print(poi_keyword, coordinates)
    cc = cc + 1
    pois_ret={}
    pois_ret['poiKey'] = 'poi_'+str(cc)
    pois_ret['poiKeyword'] = 'res_'+poi_keyword

    poiGeoJSON={}
    features=[]
    feature={}
    feature['type']='Feature'
    feature['properties']={}
    feature['geometry']={}
    feature['geometry']['type']='MultiPoint'
    coordinatestmp=[]
    for point in coordinates:        
        coordinatestmp.append([point])
    feature['geometry']['coordinates']=[coordinates]
    features.append(feature)

    poiGeoJSON['type']='FeatureCollection'
    poiGeoJSON['features']=features

    pois_ret['poiGeoJSON'] = poiGeoJSON
    pois_ret['poiName'] = 'res_'+poi_keyword
    pois_ret['poiNameI18n'] = 'res_'+poi_keyword
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
