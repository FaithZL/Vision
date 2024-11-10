
import cpplibs
import os
from cpplibs import ocapi
from cpplibs.ocapi import *
from cpplibs import vsapi
from cpplibs.vsapi import *
import numpy as np

def main():
    
    # v.push_back_(np.ones(2, dtype=np.float32))
    
    v = cpplibs.PyArray(Ray)
    v.push_back(Ray(float3(1,2,8), float3(1,2,8)))
    v.push_back(Ray(float3(3,6,8),float3(7,2,8)))
    print(v[0])
    print(v[1])
    v[0] = v[1]
    print(v.size())
    # v.pop_back()
    v.resize(10)
    print(v.size())
    
    print(v[0])
    print(v[1])
    print(v[2])
    return

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
    cpplibs.init_context("cuda")





    print("wocao")
    # t = Type.from_desc(float2().desc())
    # print(t.description())
    # print(t.name())
    # print(as_float(as_uint(2.0)))
    # print(as_float(as_uint(2.0)))
    # print(as_float(as_uint(2.0)))
    buffer = Bufferfloat2.create(2)
    # print(buffer.size())
    buffer.upload([float2(3, 4), float2(10, 11)])

    af = Arrayfloat2()
    af.resize(5)

    print(af)
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
    return

    arr = np.array([[1.0, 5.5], [5,9]], dtype=np.float32)
    # arr = [1.0, 5.5]

    buffer.download(arr)
    print(arr)
    print(buffer.download())


    # a = arr[1]

    # # buffer.download()

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

    tex.upload(iaar)

    tarr = np.ones(1024 **2 * 4, dtype=np.uint8)

    tex.download(tarr)

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