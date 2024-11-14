
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
    cpplibs.init_context("cuda")

    hit = uint2(420000000)
    h2 = uint2(420000000)
    lst = [hit, h2]
    print(lst)
    b = cpplibs.to_bytes(lst)
    print(b)
    l = cpplibs.list_from_bytes(uint2, b)
    print(l)

    return 

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