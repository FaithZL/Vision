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
print(f2.xyx * f2.xyx)

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
d = vision.create_device("cuda")
d.create_accel()
t = Type.from_desc(float2().desc())
print(t.description())
print(t.name())
print(as_float(as_uint(2.0)))
print(as_float(as_uint(2.0)))
print(as_float(as_uint(2.0)))