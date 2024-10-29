import vision

print(vision.ocapi.add(1,2))
print(vision.ocapi.sub(1,2))
f2 = vision.ocapi.float2(1,2)
print(f2)
f2[0] = 3.88999
print(f2.x)
print(f2.xx)
f2.xy = vision.ocapi.float2(5,6)
print(f2.xx)
print(f2.xy)
print(f2.yx)
# print(mc)