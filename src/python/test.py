import vision
import os
from vision import ocapi
from vision.ocapi import *

f2 = vision.ocapi.float2(1,2)
print(f2)
f2[0] = 3.88999
print(f2.x)
print(f2.xx)
f2.xy = vision.ocapi.float2(5,6)
print(f2.xx)
print(f2.xy)
print(f2.xyx)

f3 = f2.xyx

print(vision.ocapi.lerp(f3 * 0 , f3 / 2 , f3))
print(vision.ocapi.make_bool3(f3))
print(as_float3(ocapi.as_uint3(f3)))

m = float2x2()
m[0] = make_float2(1,6)

print(m)
print(float3([1,22,0]))
print(float2x2([float2(1,2),float2(3,4)]))
print(inverse(inverse(float2x2([float2(1,2),float2(3,4)]))))
print(make_float2x2([(1,2),(3,5)]) * 2)
print(make_float2x2([(1,2),(3,5)]) * make_float2x2([(1,2),(3,5)]))
print(make_float2x2([(1,2),(3,5)]) * make_float2x2([(1,2),(3,5)])[1])
print(make_float2x2(make_float2x2([(1,2),(3,5)])).clone())
print(float4x4(5))

# d = Device.create("cuda", vision.package_path)
# print("------------")
# print(d)


# os.add_dll_directory("C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.0/bin/")

# 
# 根据操作系统分隔符分割路径
if os.name == 'nt':  # Windows
    separator = ';'
else:  # Unix/Linux/Mac
    separator = ':'

# 分割路径并打印每个路径
# print("System PATH:")
# for path in path_variable.split(separator):
#     print(path)
#     if os.path.exists(path):
#         os.add_dll_directory(path)

# dll = ctypes.CDLL("D:/work/engines/Vision/src/python/vision/ocarina-backend-cuda.dll")
d = vision.create_device("cuda")
acc = d.create_accel()

# print(d, accel)



# load_lib("KERNEL32.dll")
# load_lib("ADVAPI32.dll")
# load_lib("MSVCP140D.dll")
# load_lib("CFGMGR32.dll")
# load_lib("VCRUNTIME140D.dll")
# load_lib("VCRUNTIME140_1D.dll")
# load_lib("ucrtbased.dll")
# load_lib("nvcuda.dll")
# load_lib("cudart64_12.dll")
# load_lib("nvrtc64_120_0.dll")


# load_lib("D:/work/engines/Vision/src/python/vision/ocarina-ast.dll")
# load_lib("D:/work/engines/Vision/src/python/vision/ocarina-core.dll")
# load_lib("D:/work/engines/Vision/src/python/vision/ocarina-rhi.dll")
# load_lib("D:/work/engines/Vision/src/python/vision/ocarina-dsl.dll")
# load_lib("D:/work/engines/Vision/src/python/vision/spdlogd.dll")
# load_lib("D:/work/engines/Vision/src/python/vision/ocarina-generator.dll")
# load_lib("D:/work/engines/Vision/src/python/vision/ocarina-util.dll")
print("wic")
# load_lib("D:/work/engines/Vision/src/python/vision/ocarina-backend-cuda.dll")
# dll = ctypes.CDLL("C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.0/bin/cudart64_12.dll")

# ctypes.CDLL("KERNEL32.dll")
# ctypes.CDLL("ADVAPI32.dll")
# ctypes.CDLL("MSVCP140D.dll")
# ctypes.CDLL("CFGMGR32.dll")
# ctypes.CDLL("VCRUNTIME140D.dll")
# ctypes.CDLL("VCRUNTIME140_1D.dll")
# ctypes.CDLL("ucrtbased.dll")
# ctypes.CDLL("nvcuda.dll")
# ctypes.CDLL("cudart64_12.dll")
# ctypes.CDLL("nvrtc64_120_0.dll")