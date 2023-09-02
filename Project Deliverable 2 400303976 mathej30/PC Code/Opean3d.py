#Jibin Mathew
#mathej30
#400303976
#Serical Comminication

import numpy as np
import open3d as o3d

# Load point cloud
pcd = o3d.io.read_point_cloud("data.xyz", format='xyz')
o3d.visualization.draw_geometries([pcd])

point = 1
lines = []
for i in range(10):
    for x in range(32):
        point = point + 1
        lines.append([point, point + 1])

point = 0
step = 32

#Line Plot
for i in range(10):
    for x in range(32):
        point = point + 1
        lines.append([point, point + step])

line_set = o3d.geometry.LineSet(points=o3d.utility.Vector3dVector(np.asarray(pcd.points)), lines=o3d.utility.Vector2iVector(lines))
#line_set.paint_uniform_color([1, 0, 0])
o3d.visualization.draw_geometries([line_set])
