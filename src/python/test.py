
import cpplibs
import os
from cpplibs import ocapi
from cpplibs.ocapi import *
from cpplibs import vsapi
from cpplibs.vsapi import *
import numpy as np

class Array:
    def __init__(self, _type, num):
        self._type = _type
        self.__lst = [_type() for i in range(num)]
        
    def __getitem__(self, index):
        return self.__lst[index]
    
    def __setitem__(self, index, value):
        self.__lst[index] = value
        
    def __len__(self):
        return len(self.__lst)

    def __repr__(self):
        return repr(self.__lst)
    
    def desc(self):
        return f'array<{cpplibs.desc(self._type)},{len(self)}>'
    
def main():
    f3 = Array(float2, 2)
    f3[0] = make_float2(2,3)
    print(f3.desc())
    return
    cpplibs.init_context("cuda")
    ocapi.device().init_rtx()
    
    f3b = cpplibs.PyBuffer(float3, 4)
    i3b = cpplibs.PyBuffer(uint3, 2)
    
    mesh = cpplibs.PyMesh(f3b, i3b)
    accel = Accel()
    accel.add_instance(mesh, float4x4())
    
    # print(accel.triangle_num())
    # ocapi.stream().add(mesh.build_bvh()).add(accel.build_bvh()).sync().commit()
    
    v = cpplibs.DynamicArray(Ray)
    v.push_back(Ray(make_float3(12,3,4),make_float3(12,3,4)))
    v.push_back(Ray(make_float3(12,3,18),make_float3(12,3,4)))
    print(v[0])
    print(v[1])
    print(v.size())
    bb = cpplibs.PyBuffer(Ray, 2)
    ocapi.stream().add(bb.upload(v.impl().as_float_array_t()))
    # v.pop_back()
    print(v)
    db = cpplibs.DynamicArray(Ray)
    db.resize(2)
    ocapi.stream().add(bb.download(db.impl().as_float_array_t())).sync().commit()

    print(db)
    # print(v[0])
    # print(v[1])
    f2 = cpplibs.ocapi.float2(1,2)
    print(f2)
    f2[0] = 3.88999
    print(f2.x)
    print(f2.xx)
    f2.xy = cpplibs.ocapi.float2(5,6)
    print(f2.xx)
    print(f2.xy)
    print(f2.xyx * f2.xyx)

    f3 = f2.xyx

    print(cpplibs.ocapi.lerp(f3 * 0 , f3 / 2 , f3))
    print(cpplibs.ocapi.make_bool3(f3))
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
    





    print("wocao")
    # t = Type.from_desc(float2().desc())
    # print(t.description())
    # print(t.name())
    # print(as_float(as_uint(2.0)))
    # print(as_float(as_uint(2.0)))
    # print(as_float(as_uint(2.0)))
    buffer = cpplibs.PyBuffer(float, 4)
    # print(buffer.size())
    tmp = np.array([3,4, 4,5], dtype=np.float32)
    # buffer.upload_immediately(tmp)
    ocapi.stream().add(buffer.upload(tmp))

    # af.push_back(float2(5.5, 10000))
    # af.push_back(float2(5.5, 999))
    # print(af[0])
    # print(af)
    # af.clear()
    # print(af)
    # # print(Arrayfloat())
    
    f3 = Ray(float3(1), float3(6))
    
    print(f3.to_floats())
    print(Ray.from_floats(f3.to_floats()))

    arr = np.array([1.0, 5.5, 5,9], dtype=np.float32)
    # arr = [1.0, 5.5]

    # buffer.download_immediately(arr)
    ocapi.stream().add(buffer.download(arr)).sync().commit()
    print(arr)
    print(buffer.download_immediately())
    # return


    # a = arr[1]

    # # buffer.download_immediately()

    # # print(a2)

    # # lst = []
    # # vsapi.test(lst)

    # print(lst)
    # th = TriangleHit()
    # th.bary = float3(1,2,3).xz
    # print(Ray(float3(1,2,3), float3(1,2,5)).origin)
    buffer = None

    import traceback

    # import ast_parser

    # d = ocapi.device()


    def on_mouse(*arg):
        print(*arg)
        
        
        
        
    # fn = os.getcwd() + "/res/render_scene/cbox/dispersion-hero-2000.exr"
    fn = os.getcwd() + "/res/render_scene/cbox/dispersion-hero.png"

    image = Image.load(fn, ColorSpace.LINEAR)

    iaar = image.as_uchar_array_t()

    tex = Texture.create(image.resolution, image.pixel_storage)

    tex.upload_immediately(iaar)

    tarr = np.ones(1024 **2 * 4, dtype=np.uint8)

    tex.download_immediately(tarr)

    print(tarr)
    res = image.resolution

    w = Window.create(res)
    w.set_clear_color(float4(1,0,0,1))

    w.set_mouse_callback(on_mouse)

    def func(t):
        w.set_background(tarr)
        pass

    w.run(func)

    
    
main()

ocapi.exit()