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
    
def desc(type_):
    if type_ == float:
        return "float"
    elif type_ == int:
        return "int"
    else:
        return type_.desc()
    
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