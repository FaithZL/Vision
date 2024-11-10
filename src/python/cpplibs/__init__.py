from . import ocapi
from . import vsapi
import os
import numpy as np

package_path = os.path.dirname(__file__)

def to_floats(arg):
    if type(arg) == float:
        return arg
    elif type(arg) == int:
        return ocapi.as_float(arg)
    else:
        return arg.to_floats()

def from_floats(type_, arr):
    if type_ == int:
        print("wocao", arr)
        return ocapi.as_uint(arr[0])
    elif type_ == float:
        return arr[0]
    else:
        return type_.from_floats(arr)

def sizeof(type_):
    if type_ == float:
        return 4
    elif type_ == int:
        return 4
    else:
        return type_.sizeof()

class PyArray(ocapi.StructArray):
    
    def __init__(self, type_):
        super().__init__()
        self.__type = type_
    
    def push_back(self, elm):
        arr = to_floats(elm)
        super().push_back_(arr)
        
    def at(self, index):
        size = sizeof(self.__type)
        ofs = index * size
        return from_floats(self.__type, self.load(ofs, size))
        
    def size(self):
        return int(self.size_in_byte() / sizeof(self.__type))
        
    def __repr__(self):
        
        return super().__repr__()
        

def init_context(backend):
    ocapi.init_context(backend, package_path)