import vision
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