import os
import sys

current_dir = os.path.dirname(os.path.abspath(__file__))
parent_dir = os.path.dirname(current_dir)
sys.path.append(parent_dir)

import cpplibs
from cpplibs import ocapi

def sizeof(arg):
    if arg == float or type(arg) == float:
        return 4
    elif arg == int or type(arg) == int:
        return 4
    else:
        return arg.sizeof()
    
def alignof(arg):
    if arg == float or type(arg) == float:
        return 4
    elif arg == int or type(arg) == int:
        return 4
    else:
        return arg.alignof()
    
def max_member_size(arg):
    if arg == float or type(arg) == float:
        return 4
    elif arg == int or type(arg) == int:
        return 4
    else:
        return arg.max_member_size()
    
def desc(arg):
    if arg == float or type(arg) == float:
        return "float"
    elif arg == int or type(arg) == int:
        return "int"
    else:
        return arg.desc()
    
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
    if type_ == int or type(type_) == int:
        return ocapi.bytes2int(arg)
    elif type_ == float or type(type_) == float:
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

class Buffer(ocapi.ByteBuffer):
    def __init__(self, type, size):
        self.__type = type
        super().__init__(size * sizeof(self.__type))
    
    def size(self):
        return int(self.size_in_byte() / sizeof(self.__type))
    
    def upload_immediately(self, arg0):
        if type(arg0) is bytes:
            super().upload_immediately(arg0) 
        else:
            super().upload_immediately(to_bytes(arg0)) 
    
    def download_immediately(self, arg0=None):
        if arg0 == None:
            arg0 = bytes(self.size_in_byte())
            super().download_immediately(arg0)
            return arg0
        else:
            super().download_immediately(arg0)
    
    def type(self):
        return self.__type
    
class Array:
    def __init__(self, _type, num):
        self._type = _type
        self.__lst = [_type() for i in range(num)]
        
    def __call__(self):
        return Array(self._type, len(self))
        
    def __getitem__(self, index):
        return self.__lst[index]
    
    def __setitem__(self, index, value):
        self.__lst[index] = value
        
    def __len__(self):
        return len(self.__lst)
    
    def max_member_size(self):
        return sizeof(self._type())
    
    def fill(self, elm):
        for i in range(len(self)):
            self.__lst[i] = elm

    def __repr__(self):
        return repr(self.__lst)
    
    def alignof(self):
        return alignof(self._type())
    
    def to_bytes(self):
        bt = bytes()
        for elm in self.__lst:
            bt += to_bytes(elm)
        return bt
    
    def from_bytes(self, bts):
        ret = Array(self._type, len(self))
        for i in range(len(self)):
            ofs = sizeof(self._type()) * i
            ret[i] = from_bytes(self._type(), bts[ofs:])
        return ret
    
    def sizeof(self):
        return sizeof(self._type()) * len(self)
    
    def desc(self):
        return f'array<{desc(self._type)},{len(self)}>'
    
def mem_offset(offset:int, alignment:int):
    return (offset + alignment - 1) // alignment * alignment
    
class StructType:
    def __init__(self, alignment=1, **kwargs):
        for _, type_ in kwargs.items():
            alignment = max(alignment, alignof(type_()), 0)
        self.alignment = alignment
        self.member_dict = kwargs
        self.size = self.sizeof()
    
    def alignof(self):
        return self.alignment
    
    def max_member_size(self):
        size = 0
        for name, type_ in self.member_dict.items():
            size = max(size, sizeof(type_()))
        return size
    
    def offset_array(self):
        lst = []
        size = 0
        for name, type_ in self.member_dict.items():
            size = mem_offset(size, alignof(type_()))
            lst.append(size)
            size += sizeof(type_())
        return lst
    
    def from_bytes(self, bts):
        ret = self()
        size = 0
        for name, type_ in self.member_dict.items():
            size = mem_offset(size, alignof(type_()))
            ofs = size
            m_size = sizeof(type_())
            size += m_size
            setattr(ret, name, from_bytes(type_(), bts[ofs:size]))
        return ret
    
    def sizeof(self):
        size = 0
        i_max_member_size = 0
        for name, type_ in self.member_dict.items():
            size = mem_offset(size, alignof(type_()))
            size += sizeof(type_())
            m_size = max_member_size(type_())
            if m_size > i_max_member_size:
                i_max_member_size = m_size
        mod = size % i_max_member_size
        if mod != 0:
            size += i_max_member_size - mod
        return int(size)
    
    def desc(self):
        member_desc = ""
        for name, type_ in self.member_dict.items():
            setattr(self, name, type_())
            member_desc += "," + desc(type_())
        return f"struct<PyStruct,{self.alignment},false,false" + member_desc + ">"
    
    def __call__(self):
        Type = self
        class Struct:
            def __init__(self, member_dict):
                md = {}
                for name, type_ in member_dict.items():
                    md[name] = type_()
                self.__dict__.update(md)
            
            def __repr__(self):
                return repr(self.__dict__)
            
            def type(self):
                return Type

            def to_byte(self):
                ret = bytearray(Type.sizeof())
                ofs_array = Type.offset_array()
                for i, elm in enumerate(self.__dict__.values()):
                    bt = to_bytes(elm)
                    ofs = ofs_array[i]
                    for k, b in enumerate(bt):
                        ret[ofs + k] = b
                return bytes(ret)
            
            def __setattr__(self, name, value):
                assert name in self.__dict__
                self.__dict__[name] = value
            
            def __getattr__(self, name):
                return getattr(Type, name)
            
        return Struct(self.member_dict)

def test():
    hit_t = StructType(inst=int,bary=ocapi.float2, a2=Array(int, 2))
    arr = Array(ocapi.float2, 10)
    arr.fill(ocapi.float2(1))
    print(arr)
    b = arr.to_bytes()
    arr.fill(ocapi.float2(3))
    print(arr)
    print(arr.from_bytes(b))
    # return
    hit = hit_t()
    hit.bary = ocapi.float2(3,6)
    hit.inst = 150
    hit.a2[0] = 5
    hit.a2[1] = 9
    bts = hit.to_byte()
    print(hit)
    print(hit_t.from_bytes(bts))
    # print(hit.offset_array())
    
    
    
    # print(len(hit.to_byte()))

if __name__ == "__main__":
    test()