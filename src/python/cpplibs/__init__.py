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

class StructArray(ocapi.StructArrayImpl):    
    def __init__(self, type_):
        super().__init__()
        self.__type = type_
    
    def push_back(self, elm):
        arr = to_floats(elm)
        super().push_back_(arr)
        
    def at(self, index):
        return self[index]
    
    def pop_back(self):
        size = int(sizeof(self.__type))
        super().pop_back_(size)
        
    def size(self):
        return int(super().size_in_byte() / sizeof(self.__type))
    
    def resize(self, num):
        super().resize_(num * sizeof(self.__type))
        
    def __getitem__(self, index):
        assert (index < self.size())
        size = sizeof(self.__type)
        ofs = index * size
        return from_floats(self.__type, super().load(ofs, size))
    
    def __setitem__(self, index, elm):
        assert (index < self.size())
        size = sizeof(self.__type)
        ofs = index * size
        super().store(ofs, to_floats(elm))
        
    def __repr__(self):
        ret = "["
        for i in range(self.size()):
            ret += repr(self[i]) + ","
            if (i > 100):
                ret += "......" + repr(self[self.size() - 1])
                break
        return ret + "]"
        
class PyArray:
    def __init__(self, type_):
        if type_ == int:
            self.__impl = ocapi.Arrayint()
        elif type_ == float:
            self.__impl = ocapi.Arrayfloat()
        else:
            self.__impl = StructArray(type_)

    def __getitem__(self, index):
        return self.__impl[index]
    
    def __setitem__(self, index, elm):
        self.__impl[index] = elm

    def __getattr__(self, name):
        return getattr(self.__impl, name)
    
    def impl(self):
        return self.__impl
    
    def __repr__(self):
        return repr(self.__impl)
    

class StructBuffer(ocapi.ByteBuffer):
    def __init__(self, type, size):
        self.__type = type
        super().__init__(size * sizeof(self.__type))
    
    def size(self):
        return int(self.size_in_byte() / sizeof(self.__type))
    
    def type(self):
        return self.__type

class PyBuffer:
    def __init__(self, type_, size):
        self.__type = type_
        if type_ == int:
            self.__impl = ocapi.Bufferint(size)
        elif type_ == float:
            self.__impl = ocapi.Bufferfloat(size)
        else:
            self.__impl = StructBuffer(type_, size)
    
    def download(self, array=None):
        if array is None:
            ret = PyArray(self.__type)
            ret.resize(self.__impl.size())
            self.download(ret.as_float_array_t())
            return ret
        else:
            self.__impl.download(array)
        
    def upload(self, array):
        self.__impl.upload(array)

def init_context(backend):
    ocapi.init_context(backend, package_path)