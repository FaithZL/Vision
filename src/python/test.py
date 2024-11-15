
import cpplibs
import os
from cpplibs import ocapi
from cpplibs.ocapi import *
from cpplibs import vsapi
from cpplibs.vsapi import *
import numpy as np
import oc_type



    
def main():
    # hit = hit_t()
    # print(hit)
    
    
    # hit.bary = float2(1)
    # print(hit)
    
    # return
    cpplibs.init_context("cuda")
    
    func = Function.push(FunctionTag.KERNEL)

    hit = uint2(420000000)
    h2 = uint2(420000000)
    lst = [hit, h2]
    print(lst)
    b = oc_type.to_bytes(lst)
    print(b)
    l = oc_type.list_from_bytes(uint2, b)
    print(l)

    buffer = oc_type.Buffer(uint2, 2)
    
    buffer.upload_immediately(lst)
    
    print(oc_type.list_from_bytes(uint2,buffer.download_immediately()))
    # return

    def on_mouse(*arg):
        print(*arg)
        
        
    # fn = os.getcwd() + "/res/render_scene/cbox/dispersion-hero-2000.exr"
    fn = os.getcwd() + "/res/render_scene/cbox/dispersion-hero.png"

    image = Image.load(fn, ColorSpace.LINEAR)

    iaar = image.as_uchar_array()
    # iaar = image.as_float_array()

    res = image.resolution

    w = Window.create(res)
    w.set_clear_color(float4(1,0,0,1))

    w.set_mouse_callback(on_mouse)

    def func(t):
        w.set_background(iaar)
    w.run(func)

    
    
main()

ocapi.exit()