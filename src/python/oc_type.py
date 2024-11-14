import cpplibs
from cpplibs import ocapi

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
        
    def __getitem__(self, index):
        return self.__lst[index]
    
    def __setitem__(self, index, value):
        self.__lst[index] = value
        
    def __len__(self):
        return len(self.__lst)

    def __repr__(self):
        return repr(self.__lst)
    
    def desc(self):
        return f'array<{desc(self._type)},{len(self)}>'

class Struct:
    def __init__(self, alignment, member_dict):
        self.alignment = alignment
        for name, type_ in member_dict.items():
            setattr(self, name, type_())
    
    def __repr__(self):
        return repr(self.__dict__)
    
    def desc(self):
        return f"struct<{self.alignment}>"
    
class StructType:
    def __init__(self, alignment=1, **kwargs):
        # for _, type_ in kwargs.items():
        #     alignment = max(alignment, oc_type.alignof(type_), 0)
        self.alignment = alignment
        self.member_dict = kwargs
    
    def __call__(self):
        class S:
            def __init__(self, alignment, member_dict):
                self.alignment = alignment
                for name, type_ in member_dict.items():
                    setattr(self, name, type_())
            
            def __repr__(self):
                return repr(self.__dict__)
            
            def desc(self):
                return f"struct<{self.alignment}>"
        return S

def test():
    hit_t = StructType(inst_id=int,bary=ocapi.float2)
    print(Array(ocapi.float2, 10).desc())

if __name__ == "__main__":
    test()