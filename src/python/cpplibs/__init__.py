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

class StructArray:    
    def __init__(self, type_):
        self.__type = type_
        self.__impl = ocapi.StructArrayImpl()
    
    def push_back(self, elm):
        arr = to_floats(elm)
        self.__impl.push_back_(arr)
        
    def at(self, index):
        return self[index]
    
    def pop_back(self):
        size = int(sizeof(self.__type))
        self.__impl.pop_back_(size)
        
    def size(self):
        return int(self.__impl.size_in_byte() / sizeof(self.__type))
    
    def resize(self, num):
        self.__impl.resize_(num * sizeof(self.__type))
        
    def __getitem__(self, index):
        assert (index < self.size())
        size = sizeof(self.__type)
        ofs = index * size
        return from_floats(self.__type, self.__impl.load(ofs, size))
    
    def __setitem__(self, index, elm):
        assert (index < self.size())
        size = sizeof(self.__type)
        ofs = index * size
        self.__impl.store(ofs, to_floats(elm))
        
    def __repr__(self):
        ret = "["
        for i in range(self.size()):
            ret += str(self[i]) + ","
            if (i > 100):
                ret += "......" + str(self[self.size() - 1])
                break
        
        return ret + "]"
        

def init_context(backend):
    ocapi.init_context(backend, package_path)