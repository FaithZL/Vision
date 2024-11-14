from . import ocapi
from . import vsapi
import os
import numpy as np
from . import _type

package_path = os.path.dirname(__file__)

def sizeof(type_):
    if type_ == float:
        return 4
    elif type_ == int:
        return 4
    else:
        return type_.sizeof()
    
def alignof(type_):
    if type_ == float:
        return 4
    elif type_ == int:
        return 4
    else:
        return type_.alignof()
    
def list_to_bytes(lst):
    ret = b''
    for elm in lst:
        ret += to_bytes(elm)
    return ret
    
def to_bytes(arg):
    if type(arg) in [int, float]:
        return ocapi.to_bytes(arg)
    elif type(arg) == list:
        return list_to_bytes(arg)
    else:
        return arg.to_bytes()
    
def from_bytes(type_, arg):
    if type_ == int:
        return ocapi.bytes2int(arg)
    elif type_ == float:
        return ocapi.bytes2float(arg)
    else:
        return type_.from_bytes(arg)
    
def list_from_bytes(type_, bytes):
    lst = []
    size = sizeof(type_)
    count = len(bytes) / size
    for index in range(int(count)):
        offset = index * size
        bt = bytes[offset : offset + size]
        elm = from_bytes(type_, bt)
        lst.append(elm)
    return lst
    
def desc(arg):
    if arg == float or type(arg) == float:
        return "float"
    elif arg == int or type(arg) == int:
        return "int"
    else:
        return arg.desc()
    
class Buffer(ocapi.ByteBuffer):
    def __init__(self, type, size):
        self.__type = type
        super().__init__(size * sizeof(self.__type))
    
    def size(self):
        return int(self.size_in_byte() / sizeof(self.__type))
    
    def type(self):
        return self.__type

def init_context(backend):
    ocapi.init_context(backend, package_path)