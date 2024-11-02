from __future__ import annotations
import pybind11_stubgen.typing_ext
import typing
__all__ = ['Accel', 'Device', 'RHIResource', 'abs', 'acos', 'acosh', 'all', 'any', 'as_float', 'as_float2', 'as_float3', 'as_float4', 'as_int', 'as_int2', 'as_int3', 'as_int4', 'as_uint', 'as_uint2', 'as_uint3', 'as_uint4', 'asin', 'asinh', 'atan', 'atan2', 'atanh', 'bool2', 'bool3', 'bool4', 'ceil', 'clamp', 'concepts_Noncopyable', 'cos', 'cosh', 'cross', 'degrees', 'determinant', 'distance', 'distance_squared', 'dot', 'exp', 'exp2', 'float2', 'float2x2', 'float2x3', 'float2x4', 'float3', 'float3x2', 'float3x3', 'float3x4', 'float4', 'float4x2', 'float4x3', 'float4x4', 'floor', 'fma', 'fract', 'int2', 'int3', 'int4', 'inverse', 'isinf', 'isnan', 'length', 'length_squared', 'lerp', 'load_lib', 'log', 'log10', 'log2', 'make_bool2', 'make_bool3', 'make_bool4', 'make_float2', 'make_float2x2', 'make_float2x3', 'make_float2x4', 'make_float3', 'make_float3x2', 'make_float3x3', 'make_float3x4', 'make_float4', 'make_float4x2', 'make_float4x3', 'make_float4x4', 'make_int2', 'make_int3', 'make_int4', 'make_uint2', 'make_uint3', 'make_uint4', 'max', 'min', 'none', 'normalize', 'pow', 'radians', 'rcp', 'round', 'sign', 'sin', 'sinh', 'sqr', 'sqrt', 'tan', 'tanh', 'transpose', 'uint2', 'uint3', 'uint4']
class Accel(RHIResource):
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
class Device(concepts_Noncopyable):
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    @staticmethod
    def create(arg0: str, arg1: str) -> Device:
        ...
    def create_accel(self) -> Accel:
        ...
class RHIResource:
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
class _VectorStoragebool2:
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
class _VectorStoragebool3:
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
class _VectorStoragebool4:
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
class _VectorStoragefloat2:
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
class _VectorStoragefloat3:
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
class _VectorStoragefloat4:
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
class _VectorStorageint2:
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
class _VectorStorageint3:
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
class _VectorStorageint4:
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
class _VectorStorageuint2:
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
class _VectorStorageuint3:
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
class _VectorStorageuint4:
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
class bool2(_VectorStoragebool2):
    __hash__: typing.ClassVar[None] = None
    x: bool
    xx: bool2
    xy: bool2
    y: bool
    yx: bool2
    yy: bool2
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    def __eq__(self, arg0: bool2) -> bool2:
        ...
    def __getitem__(self, arg0: int) -> bool:
        ...
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: bool) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: bool, arg1: bool) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[bool], pybind11_stubgen.typing_ext.FixedSize(2)]) -> None:
        ...
    def __ne__(self, arg0: bool2) -> bool2:
        ...
    def __repr__(self) -> str:
        ...
    def __setitem__(self, arg0: int, arg1: bool) -> None:
        ...
    def clone(self) -> bool2:
        ...
    @property
    def desc_(self) -> str:
        ...
    @property
    def xxx(self) -> ...:
        ...
    @xxx.setter
    def xxx(*args, **kwargs):
        """
        (arg0: ocapi.bool2, arg1: ocarina::Vector<bool,3>) -> None
        """
    @property
    def xxxx(self) -> ...:
        ...
    @xxxx.setter
    def xxxx(*args, **kwargs):
        """
        (arg0: ocapi.bool2, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def xxxy(self) -> ...:
        ...
    @xxxy.setter
    def xxxy(*args, **kwargs):
        """
        (arg0: ocapi.bool2, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def xxy(self) -> ...:
        ...
    @xxy.setter
    def xxy(*args, **kwargs):
        """
        (arg0: ocapi.bool2, arg1: ocarina::Vector<bool,3>) -> None
        """
    @property
    def xxyx(self) -> ...:
        ...
    @xxyx.setter
    def xxyx(*args, **kwargs):
        """
        (arg0: ocapi.bool2, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def xxyy(self) -> ...:
        ...
    @xxyy.setter
    def xxyy(*args, **kwargs):
        """
        (arg0: ocapi.bool2, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def xyx(self) -> ...:
        ...
    @xyx.setter
    def xyx(*args, **kwargs):
        """
        (arg0: ocapi.bool2, arg1: ocarina::Vector<bool,3>) -> None
        """
    @property
    def xyxx(self) -> ...:
        ...
    @xyxx.setter
    def xyxx(*args, **kwargs):
        """
        (arg0: ocapi.bool2, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def xyxy(self) -> ...:
        ...
    @xyxy.setter
    def xyxy(*args, **kwargs):
        """
        (arg0: ocapi.bool2, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def xyy(self) -> ...:
        ...
    @xyy.setter
    def xyy(*args, **kwargs):
        """
        (arg0: ocapi.bool2, arg1: ocarina::Vector<bool,3>) -> None
        """
    @property
    def xyyx(self) -> ...:
        ...
    @xyyx.setter
    def xyyx(*args, **kwargs):
        """
        (arg0: ocapi.bool2, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def xyyy(self) -> ...:
        ...
    @xyyy.setter
    def xyyy(*args, **kwargs):
        """
        (arg0: ocapi.bool2, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yxx(self) -> ...:
        ...
    @yxx.setter
    def yxx(*args, **kwargs):
        """
        (arg0: ocapi.bool2, arg1: ocarina::Vector<bool,3>) -> None
        """
    @property
    def yxxx(self) -> ...:
        ...
    @yxxx.setter
    def yxxx(*args, **kwargs):
        """
        (arg0: ocapi.bool2, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yxxy(self) -> ...:
        ...
    @yxxy.setter
    def yxxy(*args, **kwargs):
        """
        (arg0: ocapi.bool2, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yxy(self) -> ...:
        ...
    @yxy.setter
    def yxy(*args, **kwargs):
        """
        (arg0: ocapi.bool2, arg1: ocarina::Vector<bool,3>) -> None
        """
    @property
    def yxyx(self) -> ...:
        ...
    @yxyx.setter
    def yxyx(*args, **kwargs):
        """
        (arg0: ocapi.bool2, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yxyy(self) -> ...:
        ...
    @yxyy.setter
    def yxyy(*args, **kwargs):
        """
        (arg0: ocapi.bool2, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yyx(self) -> ...:
        ...
    @yyx.setter
    def yyx(*args, **kwargs):
        """
        (arg0: ocapi.bool2, arg1: ocarina::Vector<bool,3>) -> None
        """
    @property
    def yyxx(self) -> ...:
        ...
    @yyxx.setter
    def yyxx(*args, **kwargs):
        """
        (arg0: ocapi.bool2, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yyxy(self) -> ...:
        ...
    @yyxy.setter
    def yyxy(*args, **kwargs):
        """
        (arg0: ocapi.bool2, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yyy(self) -> ...:
        ...
    @yyy.setter
    def yyy(*args, **kwargs):
        """
        (arg0: ocapi.bool2, arg1: ocarina::Vector<bool,3>) -> None
        """
    @property
    def yyyx(self) -> ...:
        ...
    @yyyx.setter
    def yyyx(*args, **kwargs):
        """
        (arg0: ocapi.bool2, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yyyy(self) -> ...:
        ...
    @yyyy.setter
    def yyyy(*args, **kwargs):
        """
        (arg0: ocapi.bool2, arg1: ocarina::Vector<bool,4>) -> None
        """
class bool3(_VectorStoragebool3):
    __hash__: typing.ClassVar[None] = None
    x: bool
    xx: bool2
    xxx: bool3
    xxy: bool3
    xxz: bool3
    xy: bool2
    xyx: bool3
    xyy: bool3
    xyz: bool3
    xz: bool2
    xzx: bool3
    xzy: bool3
    xzz: bool3
    y: bool
    yx: bool2
    yxx: bool3
    yxy: bool3
    yxz: bool3
    yy: bool2
    yyx: bool3
    yyy: bool3
    yyz: bool3
    yz: bool2
    yzx: bool3
    yzy: bool3
    yzz: bool3
    z: bool
    zx: bool2
    zxx: bool3
    zxy: bool3
    zxz: bool3
    zy: bool2
    zyx: bool3
    zyy: bool3
    zyz: bool3
    zz: bool2
    zzx: bool3
    zzy: bool3
    zzz: bool3
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    def __eq__(self, arg0: bool3) -> bool3:
        ...
    def __getitem__(self, arg0: int) -> bool:
        ...
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: bool) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: bool, arg1: bool, arg2: bool) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[bool], pybind11_stubgen.typing_ext.FixedSize(3)]) -> None:
        ...
    def __ne__(self, arg0: bool3) -> bool3:
        ...
    def __repr__(self) -> str:
        ...
    def __setitem__(self, arg0: int, arg1: bool) -> None:
        ...
    def clone(self) -> bool3:
        ...
    @property
    def desc_(self) -> str:
        ...
    @property
    def xxxx(self) -> ...:
        ...
    @xxxx.setter
    def xxxx(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def xxxy(self) -> ...:
        ...
    @xxxy.setter
    def xxxy(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def xxxz(self) -> ...:
        ...
    @xxxz.setter
    def xxxz(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def xxyx(self) -> ...:
        ...
    @xxyx.setter
    def xxyx(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def xxyy(self) -> ...:
        ...
    @xxyy.setter
    def xxyy(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def xxyz(self) -> ...:
        ...
    @xxyz.setter
    def xxyz(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def xxzx(self) -> ...:
        ...
    @xxzx.setter
    def xxzx(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def xxzy(self) -> ...:
        ...
    @xxzy.setter
    def xxzy(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def xxzz(self) -> ...:
        ...
    @xxzz.setter
    def xxzz(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def xyxx(self) -> ...:
        ...
    @xyxx.setter
    def xyxx(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def xyxy(self) -> ...:
        ...
    @xyxy.setter
    def xyxy(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def xyxz(self) -> ...:
        ...
    @xyxz.setter
    def xyxz(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def xyyx(self) -> ...:
        ...
    @xyyx.setter
    def xyyx(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def xyyy(self) -> ...:
        ...
    @xyyy.setter
    def xyyy(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def xyyz(self) -> ...:
        ...
    @xyyz.setter
    def xyyz(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def xyzx(self) -> ...:
        ...
    @xyzx.setter
    def xyzx(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def xyzy(self) -> ...:
        ...
    @xyzy.setter
    def xyzy(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def xyzz(self) -> ...:
        ...
    @xyzz.setter
    def xyzz(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def xzxx(self) -> ...:
        ...
    @xzxx.setter
    def xzxx(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def xzxy(self) -> ...:
        ...
    @xzxy.setter
    def xzxy(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def xzxz(self) -> ...:
        ...
    @xzxz.setter
    def xzxz(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def xzyx(self) -> ...:
        ...
    @xzyx.setter
    def xzyx(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def xzyy(self) -> ...:
        ...
    @xzyy.setter
    def xzyy(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def xzyz(self) -> ...:
        ...
    @xzyz.setter
    def xzyz(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def xzzx(self) -> ...:
        ...
    @xzzx.setter
    def xzzx(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def xzzy(self) -> ...:
        ...
    @xzzy.setter
    def xzzy(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def xzzz(self) -> ...:
        ...
    @xzzz.setter
    def xzzz(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yxxx(self) -> ...:
        ...
    @yxxx.setter
    def yxxx(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yxxy(self) -> ...:
        ...
    @yxxy.setter
    def yxxy(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yxxz(self) -> ...:
        ...
    @yxxz.setter
    def yxxz(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yxyx(self) -> ...:
        ...
    @yxyx.setter
    def yxyx(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yxyy(self) -> ...:
        ...
    @yxyy.setter
    def yxyy(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yxyz(self) -> ...:
        ...
    @yxyz.setter
    def yxyz(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yxzx(self) -> ...:
        ...
    @yxzx.setter
    def yxzx(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yxzy(self) -> ...:
        ...
    @yxzy.setter
    def yxzy(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yxzz(self) -> ...:
        ...
    @yxzz.setter
    def yxzz(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yyxx(self) -> ...:
        ...
    @yyxx.setter
    def yyxx(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yyxy(self) -> ...:
        ...
    @yyxy.setter
    def yyxy(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yyxz(self) -> ...:
        ...
    @yyxz.setter
    def yyxz(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yyyx(self) -> ...:
        ...
    @yyyx.setter
    def yyyx(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yyyy(self) -> ...:
        ...
    @yyyy.setter
    def yyyy(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yyyz(self) -> ...:
        ...
    @yyyz.setter
    def yyyz(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yyzx(self) -> ...:
        ...
    @yyzx.setter
    def yyzx(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yyzy(self) -> ...:
        ...
    @yyzy.setter
    def yyzy(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yyzz(self) -> ...:
        ...
    @yyzz.setter
    def yyzz(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yzxx(self) -> ...:
        ...
    @yzxx.setter
    def yzxx(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yzxy(self) -> ...:
        ...
    @yzxy.setter
    def yzxy(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yzxz(self) -> ...:
        ...
    @yzxz.setter
    def yzxz(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yzyx(self) -> ...:
        ...
    @yzyx.setter
    def yzyx(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yzyy(self) -> ...:
        ...
    @yzyy.setter
    def yzyy(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yzyz(self) -> ...:
        ...
    @yzyz.setter
    def yzyz(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yzzx(self) -> ...:
        ...
    @yzzx.setter
    def yzzx(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yzzy(self) -> ...:
        ...
    @yzzy.setter
    def yzzy(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def yzzz(self) -> ...:
        ...
    @yzzz.setter
    def yzzz(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def zxxx(self) -> ...:
        ...
    @zxxx.setter
    def zxxx(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def zxxy(self) -> ...:
        ...
    @zxxy.setter
    def zxxy(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def zxxz(self) -> ...:
        ...
    @zxxz.setter
    def zxxz(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def zxyx(self) -> ...:
        ...
    @zxyx.setter
    def zxyx(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def zxyy(self) -> ...:
        ...
    @zxyy.setter
    def zxyy(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def zxyz(self) -> ...:
        ...
    @zxyz.setter
    def zxyz(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def zxzx(self) -> ...:
        ...
    @zxzx.setter
    def zxzx(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def zxzy(self) -> ...:
        ...
    @zxzy.setter
    def zxzy(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def zxzz(self) -> ...:
        ...
    @zxzz.setter
    def zxzz(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def zyxx(self) -> ...:
        ...
    @zyxx.setter
    def zyxx(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def zyxy(self) -> ...:
        ...
    @zyxy.setter
    def zyxy(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def zyxz(self) -> ...:
        ...
    @zyxz.setter
    def zyxz(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def zyyx(self) -> ...:
        ...
    @zyyx.setter
    def zyyx(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def zyyy(self) -> ...:
        ...
    @zyyy.setter
    def zyyy(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def zyyz(self) -> ...:
        ...
    @zyyz.setter
    def zyyz(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def zyzx(self) -> ...:
        ...
    @zyzx.setter
    def zyzx(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def zyzy(self) -> ...:
        ...
    @zyzy.setter
    def zyzy(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def zyzz(self) -> ...:
        ...
    @zyzz.setter
    def zyzz(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def zzxx(self) -> ...:
        ...
    @zzxx.setter
    def zzxx(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def zzxy(self) -> ...:
        ...
    @zzxy.setter
    def zzxy(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def zzxz(self) -> ...:
        ...
    @zzxz.setter
    def zzxz(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def zzyx(self) -> ...:
        ...
    @zzyx.setter
    def zzyx(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def zzyy(self) -> ...:
        ...
    @zzyy.setter
    def zzyy(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def zzyz(self) -> ...:
        ...
    @zzyz.setter
    def zzyz(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def zzzx(self) -> ...:
        ...
    @zzzx.setter
    def zzzx(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def zzzy(self) -> ...:
        ...
    @zzzy.setter
    def zzzy(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
    @property
    def zzzz(self) -> ...:
        ...
    @zzzz.setter
    def zzzz(*args, **kwargs):
        """
        (arg0: ocapi.bool3, arg1: ocarina::Vector<bool,4>) -> None
        """
class bool4(_VectorStoragebool4):
    __hash__: typing.ClassVar[None] = None
    w: bool
    ww: bool2
    www: bool3
    wwww: bool4
    wwwx: bool4
    wwwy: bool4
    wwwz: bool4
    wwx: bool3
    wwxw: bool4
    wwxx: bool4
    wwxy: bool4
    wwxz: bool4
    wwy: bool3
    wwyw: bool4
    wwyx: bool4
    wwyy: bool4
    wwyz: bool4
    wwz: bool3
    wwzw: bool4
    wwzx: bool4
    wwzy: bool4
    wwzz: bool4
    wx: bool2
    wxw: bool3
    wxww: bool4
    wxwx: bool4
    wxwy: bool4
    wxwz: bool4
    wxx: bool3
    wxxw: bool4
    wxxx: bool4
    wxxy: bool4
    wxxz: bool4
    wxy: bool3
    wxyw: bool4
    wxyx: bool4
    wxyy: bool4
    wxyz: bool4
    wxz: bool3
    wxzw: bool4
    wxzx: bool4
    wxzy: bool4
    wxzz: bool4
    wy: bool2
    wyw: bool3
    wyww: bool4
    wywx: bool4
    wywy: bool4
    wywz: bool4
    wyx: bool3
    wyxw: bool4
    wyxx: bool4
    wyxy: bool4
    wyxz: bool4
    wyy: bool3
    wyyw: bool4
    wyyx: bool4
    wyyy: bool4
    wyyz: bool4
    wyz: bool3
    wyzw: bool4
    wyzx: bool4
    wyzy: bool4
    wyzz: bool4
    wz: bool2
    wzw: bool3
    wzww: bool4
    wzwx: bool4
    wzwy: bool4
    wzwz: bool4
    wzx: bool3
    wzxw: bool4
    wzxx: bool4
    wzxy: bool4
    wzxz: bool4
    wzy: bool3
    wzyw: bool4
    wzyx: bool4
    wzyy: bool4
    wzyz: bool4
    wzz: bool3
    wzzw: bool4
    wzzx: bool4
    wzzy: bool4
    wzzz: bool4
    x: bool
    xw: bool2
    xww: bool3
    xwww: bool4
    xwwx: bool4
    xwwy: bool4
    xwwz: bool4
    xwx: bool3
    xwxw: bool4
    xwxx: bool4
    xwxy: bool4
    xwxz: bool4
    xwy: bool3
    xwyw: bool4
    xwyx: bool4
    xwyy: bool4
    xwyz: bool4
    xwz: bool3
    xwzw: bool4
    xwzx: bool4
    xwzy: bool4
    xwzz: bool4
    xx: bool2
    xxw: bool3
    xxww: bool4
    xxwx: bool4
    xxwy: bool4
    xxwz: bool4
    xxx: bool3
    xxxw: bool4
    xxxx: bool4
    xxxy: bool4
    xxxz: bool4
    xxy: bool3
    xxyw: bool4
    xxyx: bool4
    xxyy: bool4
    xxyz: bool4
    xxz: bool3
    xxzw: bool4
    xxzx: bool4
    xxzy: bool4
    xxzz: bool4
    xy: bool2
    xyw: bool3
    xyww: bool4
    xywx: bool4
    xywy: bool4
    xywz: bool4
    xyx: bool3
    xyxw: bool4
    xyxx: bool4
    xyxy: bool4
    xyxz: bool4
    xyy: bool3
    xyyw: bool4
    xyyx: bool4
    xyyy: bool4
    xyyz: bool4
    xyz: bool3
    xyzw: bool4
    xyzx: bool4
    xyzy: bool4
    xyzz: bool4
    xz: bool2
    xzw: bool3
    xzww: bool4
    xzwx: bool4
    xzwy: bool4
    xzwz: bool4
    xzx: bool3
    xzxw: bool4
    xzxx: bool4
    xzxy: bool4
    xzxz: bool4
    xzy: bool3
    xzyw: bool4
    xzyx: bool4
    xzyy: bool4
    xzyz: bool4
    xzz: bool3
    xzzw: bool4
    xzzx: bool4
    xzzy: bool4
    xzzz: bool4
    y: bool
    yw: bool2
    yww: bool3
    ywww: bool4
    ywwx: bool4
    ywwy: bool4
    ywwz: bool4
    ywx: bool3
    ywxw: bool4
    ywxx: bool4
    ywxy: bool4
    ywxz: bool4
    ywy: bool3
    ywyw: bool4
    ywyx: bool4
    ywyy: bool4
    ywyz: bool4
    ywz: bool3
    ywzw: bool4
    ywzx: bool4
    ywzy: bool4
    ywzz: bool4
    yx: bool2
    yxw: bool3
    yxww: bool4
    yxwx: bool4
    yxwy: bool4
    yxwz: bool4
    yxx: bool3
    yxxw: bool4
    yxxx: bool4
    yxxy: bool4
    yxxz: bool4
    yxy: bool3
    yxyw: bool4
    yxyx: bool4
    yxyy: bool4
    yxyz: bool4
    yxz: bool3
    yxzw: bool4
    yxzx: bool4
    yxzy: bool4
    yxzz: bool4
    yy: bool2
    yyw: bool3
    yyww: bool4
    yywx: bool4
    yywy: bool4
    yywz: bool4
    yyx: bool3
    yyxw: bool4
    yyxx: bool4
    yyxy: bool4
    yyxz: bool4
    yyy: bool3
    yyyw: bool4
    yyyx: bool4
    yyyy: bool4
    yyyz: bool4
    yyz: bool3
    yyzw: bool4
    yyzx: bool4
    yyzy: bool4
    yyzz: bool4
    yz: bool2
    yzw: bool3
    yzww: bool4
    yzwx: bool4
    yzwy: bool4
    yzwz: bool4
    yzx: bool3
    yzxw: bool4
    yzxx: bool4
    yzxy: bool4
    yzxz: bool4
    yzy: bool3
    yzyw: bool4
    yzyx: bool4
    yzyy: bool4
    yzyz: bool4
    yzz: bool3
    yzzw: bool4
    yzzx: bool4
    yzzy: bool4
    yzzz: bool4
    z: bool
    zw: bool2
    zww: bool3
    zwww: bool4
    zwwx: bool4
    zwwy: bool4
    zwwz: bool4
    zwx: bool3
    zwxw: bool4
    zwxx: bool4
    zwxy: bool4
    zwxz: bool4
    zwy: bool3
    zwyw: bool4
    zwyx: bool4
    zwyy: bool4
    zwyz: bool4
    zwz: bool3
    zwzw: bool4
    zwzx: bool4
    zwzy: bool4
    zwzz: bool4
    zx: bool2
    zxw: bool3
    zxww: bool4
    zxwx: bool4
    zxwy: bool4
    zxwz: bool4
    zxx: bool3
    zxxw: bool4
    zxxx: bool4
    zxxy: bool4
    zxxz: bool4
    zxy: bool3
    zxyw: bool4
    zxyx: bool4
    zxyy: bool4
    zxyz: bool4
    zxz: bool3
    zxzw: bool4
    zxzx: bool4
    zxzy: bool4
    zxzz: bool4
    zy: bool2
    zyw: bool3
    zyww: bool4
    zywx: bool4
    zywy: bool4
    zywz: bool4
    zyx: bool3
    zyxw: bool4
    zyxx: bool4
    zyxy: bool4
    zyxz: bool4
    zyy: bool3
    zyyw: bool4
    zyyx: bool4
    zyyy: bool4
    zyyz: bool4
    zyz: bool3
    zyzw: bool4
    zyzx: bool4
    zyzy: bool4
    zyzz: bool4
    zz: bool2
    zzw: bool3
    zzww: bool4
    zzwx: bool4
    zzwy: bool4
    zzwz: bool4
    zzx: bool3
    zzxw: bool4
    zzxx: bool4
    zzxy: bool4
    zzxz: bool4
    zzy: bool3
    zzyw: bool4
    zzyx: bool4
    zzyy: bool4
    zzyz: bool4
    zzz: bool3
    zzzw: bool4
    zzzx: bool4
    zzzy: bool4
    zzzz: bool4
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    def __eq__(self, arg0: bool4) -> bool4:
        ...
    def __getitem__(self, arg0: int) -> bool:
        ...
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: bool) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: bool, arg1: bool, arg2: bool, arg3: bool) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[bool], pybind11_stubgen.typing_ext.FixedSize(4)]) -> None:
        ...
    def __ne__(self, arg0: bool4) -> bool4:
        ...
    def __repr__(self) -> str:
        ...
    def __setitem__(self, arg0: int, arg1: bool) -> None:
        ...
    def clone(self) -> bool4:
        ...
    @property
    def desc_(self) -> str:
        ...
class concepts_Noncopyable:
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
class float2(_VectorStoragefloat2):
    __hash__: typing.ClassVar[None] = None
    x: float
    xx: float2
    xy: float2
    y: float
    yx: float2
    yy: float2
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    @typing.overload
    def __add__(self, arg0: float2) -> float2:
        ...
    @typing.overload
    def __add__(self, arg0: float) -> float2:
        ...
    def __eq__(self, arg0: float2) -> ...:
        ...
    def __ge__(self, arg0: float2) -> ...:
        ...
    def __getitem__(self, arg0: int) -> float:
        ...
    def __gt__(self, arg0: float2) -> ...:
        ...
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: float) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: float, arg1: float) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(2)]) -> None:
        ...
    def __le__(self, arg0: float2) -> ...:
        ...
    def __lt__(self, arg0: float2) -> ...:
        ...
    @typing.overload
    def __mul__(self, arg0: float2) -> float2:
        ...
    @typing.overload
    def __mul__(self, arg0: float) -> float2:
        ...
    def __ne__(self, arg0: float2) -> ...:
        ...
    def __neg__(self) -> float2:
        ...
    def __radd__(self, arg0: float) -> float2:
        ...
    def __repr__(self) -> str:
        ...
    def __rmul__(self, arg0: float) -> float2:
        ...
    def __rsub__(self, arg0: float) -> float2:
        ...
    def __rtruediv__(self, arg0: float) -> float2:
        ...
    def __setitem__(self, arg0: int, arg1: float) -> None:
        ...
    @typing.overload
    def __sub__(self, arg0: float2) -> float2:
        ...
    @typing.overload
    def __sub__(self, arg0: float) -> float2:
        ...
    @typing.overload
    def __truediv__(self, arg0: float2) -> float2:
        ...
    @typing.overload
    def __truediv__(self, arg0: float) -> float2:
        ...
    def clone(self) -> float2:
        ...
    @property
    def desc_(self) -> str:
        ...
    @property
    def xxx(self) -> ...:
        ...
    @xxx.setter
    def xxx(*args, **kwargs):
        """
        (arg0: ocapi.float2, arg1: ocarina::Vector<float,3>) -> None
        """
    @property
    def xxxx(self) -> ...:
        ...
    @xxxx.setter
    def xxxx(*args, **kwargs):
        """
        (arg0: ocapi.float2, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def xxxy(self) -> ...:
        ...
    @xxxy.setter
    def xxxy(*args, **kwargs):
        """
        (arg0: ocapi.float2, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def xxy(self) -> ...:
        ...
    @xxy.setter
    def xxy(*args, **kwargs):
        """
        (arg0: ocapi.float2, arg1: ocarina::Vector<float,3>) -> None
        """
    @property
    def xxyx(self) -> ...:
        ...
    @xxyx.setter
    def xxyx(*args, **kwargs):
        """
        (arg0: ocapi.float2, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def xxyy(self) -> ...:
        ...
    @xxyy.setter
    def xxyy(*args, **kwargs):
        """
        (arg0: ocapi.float2, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def xyx(self) -> ...:
        ...
    @xyx.setter
    def xyx(*args, **kwargs):
        """
        (arg0: ocapi.float2, arg1: ocarina::Vector<float,3>) -> None
        """
    @property
    def xyxx(self) -> ...:
        ...
    @xyxx.setter
    def xyxx(*args, **kwargs):
        """
        (arg0: ocapi.float2, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def xyxy(self) -> ...:
        ...
    @xyxy.setter
    def xyxy(*args, **kwargs):
        """
        (arg0: ocapi.float2, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def xyy(self) -> ...:
        ...
    @xyy.setter
    def xyy(*args, **kwargs):
        """
        (arg0: ocapi.float2, arg1: ocarina::Vector<float,3>) -> None
        """
    @property
    def xyyx(self) -> ...:
        ...
    @xyyx.setter
    def xyyx(*args, **kwargs):
        """
        (arg0: ocapi.float2, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def xyyy(self) -> ...:
        ...
    @xyyy.setter
    def xyyy(*args, **kwargs):
        """
        (arg0: ocapi.float2, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yxx(self) -> ...:
        ...
    @yxx.setter
    def yxx(*args, **kwargs):
        """
        (arg0: ocapi.float2, arg1: ocarina::Vector<float,3>) -> None
        """
    @property
    def yxxx(self) -> ...:
        ...
    @yxxx.setter
    def yxxx(*args, **kwargs):
        """
        (arg0: ocapi.float2, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yxxy(self) -> ...:
        ...
    @yxxy.setter
    def yxxy(*args, **kwargs):
        """
        (arg0: ocapi.float2, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yxy(self) -> ...:
        ...
    @yxy.setter
    def yxy(*args, **kwargs):
        """
        (arg0: ocapi.float2, arg1: ocarina::Vector<float,3>) -> None
        """
    @property
    def yxyx(self) -> ...:
        ...
    @yxyx.setter
    def yxyx(*args, **kwargs):
        """
        (arg0: ocapi.float2, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yxyy(self) -> ...:
        ...
    @yxyy.setter
    def yxyy(*args, **kwargs):
        """
        (arg0: ocapi.float2, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yyx(self) -> ...:
        ...
    @yyx.setter
    def yyx(*args, **kwargs):
        """
        (arg0: ocapi.float2, arg1: ocarina::Vector<float,3>) -> None
        """
    @property
    def yyxx(self) -> ...:
        ...
    @yyxx.setter
    def yyxx(*args, **kwargs):
        """
        (arg0: ocapi.float2, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yyxy(self) -> ...:
        ...
    @yyxy.setter
    def yyxy(*args, **kwargs):
        """
        (arg0: ocapi.float2, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yyy(self) -> ...:
        ...
    @yyy.setter
    def yyy(*args, **kwargs):
        """
        (arg0: ocapi.float2, arg1: ocarina::Vector<float,3>) -> None
        """
    @property
    def yyyx(self) -> ...:
        ...
    @yyyx.setter
    def yyyx(*args, **kwargs):
        """
        (arg0: ocapi.float2, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yyyy(self) -> ...:
        ...
    @yyyy.setter
    def yyyy(*args, **kwargs):
        """
        (arg0: ocapi.float2, arg1: ocarina::Vector<float,4>) -> None
        """
class float2x2:
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    def __add__(self, arg0: float2x2) -> float2x2:
        ...
    def __getitem__(self, arg0: int) -> float2:
        ...
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: float) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: float2x2) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(4)]) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[float2], pybind11_stubgen.typing_ext.FixedSize(2)]) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(2)]], pybind11_stubgen.typing_ext.FixedSize(2)]) -> None:
        ...
    @typing.overload
    def __mul__(self, arg0: float) -> float2x2:
        ...
    @typing.overload
    def __mul__(self, arg0: float2) -> float2:
        ...
    @typing.overload
    def __mul__(self, arg0: float2x2) -> float2x2:
        ...
    def __neg__(self) -> float2x2:
        ...
    def __repr__(self) -> str:
        ...
    def __rmul__(self, arg0: float) -> float2x2:
        ...
    def __setitem__(self, arg0: int, arg1: float2) -> None:
        ...
    def __sub__(self, arg0: float2x2) -> float2x2:
        ...
    def __truediv__(self, arg0: float) -> float2x2:
        ...
    @typing.overload
    def clone(self) -> float2x2:
        ...
    @typing.overload
    def clone(self) -> float2x2:
        ...
    @property
    def desc_(self) -> str:
        ...
class float2x3:
    @staticmethod
    @typing.overload
    def __mul__(*args, **kwargs) -> ...:
        ...
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    def __add__(self, arg0: float2x3) -> float2x3:
        ...
    def __getitem__(self, arg0: int) -> float3:
        ...
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: float) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: float2x3) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(6)]) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[float3], pybind11_stubgen.typing_ext.FixedSize(2)]) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(3)]], pybind11_stubgen.typing_ext.FixedSize(2)]) -> None:
        ...
    @typing.overload
    def __mul__(self, arg0: float) -> float2x3:
        ...
    @typing.overload
    def __mul__(self, arg0: float2) -> float3:
        ...
    def __neg__(self) -> float2x3:
        ...
    def __repr__(self) -> str:
        ...
    def __rmul__(self, arg0: float) -> float2x3:
        ...
    def __setitem__(self, arg0: int, arg1: float3) -> None:
        ...
    def __sub__(self, arg0: float2x3) -> float2x3:
        ...
    def __truediv__(self, arg0: float) -> float2x3:
        ...
    @typing.overload
    def clone(self) -> float2x3:
        ...
    @typing.overload
    def clone(self) -> float2x3:
        ...
    @property
    def desc_(self) -> str:
        ...
class float2x4:
    @staticmethod
    @typing.overload
    def __mul__(*args, **kwargs) -> ...:
        ...
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    def __add__(self, arg0: float2x4) -> float2x4:
        ...
    def __getitem__(self, arg0: int) -> float4:
        ...
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: float) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: float2x4) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(8)]) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[float4], pybind11_stubgen.typing_ext.FixedSize(2)]) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(4)]], pybind11_stubgen.typing_ext.FixedSize(2)]) -> None:
        ...
    @typing.overload
    def __mul__(self, arg0: float) -> float2x4:
        ...
    @typing.overload
    def __mul__(self, arg0: float2) -> float4:
        ...
    def __neg__(self) -> float2x4:
        ...
    def __repr__(self) -> str:
        ...
    def __rmul__(self, arg0: float) -> float2x4:
        ...
    def __setitem__(self, arg0: int, arg1: float4) -> None:
        ...
    def __sub__(self, arg0: float2x4) -> float2x4:
        ...
    def __truediv__(self, arg0: float) -> float2x4:
        ...
    @typing.overload
    def clone(self) -> float2x4:
        ...
    @typing.overload
    def clone(self) -> float2x4:
        ...
    @property
    def desc_(self) -> str:
        ...
class float3(_VectorStoragefloat3):
    __hash__: typing.ClassVar[None] = None
    x: float
    xx: float2
    xxx: float3
    xxy: float3
    xxz: float3
    xy: float2
    xyx: float3
    xyy: float3
    xyz: float3
    xz: float2
    xzx: float3
    xzy: float3
    xzz: float3
    y: float
    yx: float2
    yxx: float3
    yxy: float3
    yxz: float3
    yy: float2
    yyx: float3
    yyy: float3
    yyz: float3
    yz: float2
    yzx: float3
    yzy: float3
    yzz: float3
    z: float
    zx: float2
    zxx: float3
    zxy: float3
    zxz: float3
    zy: float2
    zyx: float3
    zyy: float3
    zyz: float3
    zz: float2
    zzx: float3
    zzy: float3
    zzz: float3
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    @typing.overload
    def __add__(self, arg0: float3) -> float3:
        ...
    @typing.overload
    def __add__(self, arg0: float) -> float3:
        ...
    def __eq__(self, arg0: float3) -> ...:
        ...
    def __ge__(self, arg0: float3) -> ...:
        ...
    def __getitem__(self, arg0: int) -> float:
        ...
    def __gt__(self, arg0: float3) -> ...:
        ...
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: float) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: float, arg1: float, arg2: float) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(3)]) -> None:
        ...
    def __le__(self, arg0: float3) -> ...:
        ...
    def __lt__(self, arg0: float3) -> ...:
        ...
    @typing.overload
    def __mul__(self, arg0: float3) -> float3:
        ...
    @typing.overload
    def __mul__(self, arg0: float) -> float3:
        ...
    def __ne__(self, arg0: float3) -> ...:
        ...
    def __neg__(self) -> float3:
        ...
    def __radd__(self, arg0: float) -> float3:
        ...
    def __repr__(self) -> str:
        ...
    def __rmul__(self, arg0: float) -> float3:
        ...
    def __rsub__(self, arg0: float) -> float3:
        ...
    def __rtruediv__(self, arg0: float) -> float3:
        ...
    def __setitem__(self, arg0: int, arg1: float) -> None:
        ...
    @typing.overload
    def __sub__(self, arg0: float3) -> float3:
        ...
    @typing.overload
    def __sub__(self, arg0: float) -> float3:
        ...
    @typing.overload
    def __truediv__(self, arg0: float3) -> float3:
        ...
    @typing.overload
    def __truediv__(self, arg0: float) -> float3:
        ...
    def clone(self) -> float3:
        ...
    @property
    def desc_(self) -> str:
        ...
    @property
    def xxxx(self) -> ...:
        ...
    @xxxx.setter
    def xxxx(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def xxxy(self) -> ...:
        ...
    @xxxy.setter
    def xxxy(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def xxxz(self) -> ...:
        ...
    @xxxz.setter
    def xxxz(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def xxyx(self) -> ...:
        ...
    @xxyx.setter
    def xxyx(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def xxyy(self) -> ...:
        ...
    @xxyy.setter
    def xxyy(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def xxyz(self) -> ...:
        ...
    @xxyz.setter
    def xxyz(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def xxzx(self) -> ...:
        ...
    @xxzx.setter
    def xxzx(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def xxzy(self) -> ...:
        ...
    @xxzy.setter
    def xxzy(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def xxzz(self) -> ...:
        ...
    @xxzz.setter
    def xxzz(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def xyxx(self) -> ...:
        ...
    @xyxx.setter
    def xyxx(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def xyxy(self) -> ...:
        ...
    @xyxy.setter
    def xyxy(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def xyxz(self) -> ...:
        ...
    @xyxz.setter
    def xyxz(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def xyyx(self) -> ...:
        ...
    @xyyx.setter
    def xyyx(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def xyyy(self) -> ...:
        ...
    @xyyy.setter
    def xyyy(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def xyyz(self) -> ...:
        ...
    @xyyz.setter
    def xyyz(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def xyzx(self) -> ...:
        ...
    @xyzx.setter
    def xyzx(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def xyzy(self) -> ...:
        ...
    @xyzy.setter
    def xyzy(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def xyzz(self) -> ...:
        ...
    @xyzz.setter
    def xyzz(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def xzxx(self) -> ...:
        ...
    @xzxx.setter
    def xzxx(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def xzxy(self) -> ...:
        ...
    @xzxy.setter
    def xzxy(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def xzxz(self) -> ...:
        ...
    @xzxz.setter
    def xzxz(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def xzyx(self) -> ...:
        ...
    @xzyx.setter
    def xzyx(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def xzyy(self) -> ...:
        ...
    @xzyy.setter
    def xzyy(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def xzyz(self) -> ...:
        ...
    @xzyz.setter
    def xzyz(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def xzzx(self) -> ...:
        ...
    @xzzx.setter
    def xzzx(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def xzzy(self) -> ...:
        ...
    @xzzy.setter
    def xzzy(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def xzzz(self) -> ...:
        ...
    @xzzz.setter
    def xzzz(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yxxx(self) -> ...:
        ...
    @yxxx.setter
    def yxxx(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yxxy(self) -> ...:
        ...
    @yxxy.setter
    def yxxy(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yxxz(self) -> ...:
        ...
    @yxxz.setter
    def yxxz(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yxyx(self) -> ...:
        ...
    @yxyx.setter
    def yxyx(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yxyy(self) -> ...:
        ...
    @yxyy.setter
    def yxyy(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yxyz(self) -> ...:
        ...
    @yxyz.setter
    def yxyz(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yxzx(self) -> ...:
        ...
    @yxzx.setter
    def yxzx(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yxzy(self) -> ...:
        ...
    @yxzy.setter
    def yxzy(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yxzz(self) -> ...:
        ...
    @yxzz.setter
    def yxzz(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yyxx(self) -> ...:
        ...
    @yyxx.setter
    def yyxx(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yyxy(self) -> ...:
        ...
    @yyxy.setter
    def yyxy(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yyxz(self) -> ...:
        ...
    @yyxz.setter
    def yyxz(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yyyx(self) -> ...:
        ...
    @yyyx.setter
    def yyyx(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yyyy(self) -> ...:
        ...
    @yyyy.setter
    def yyyy(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yyyz(self) -> ...:
        ...
    @yyyz.setter
    def yyyz(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yyzx(self) -> ...:
        ...
    @yyzx.setter
    def yyzx(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yyzy(self) -> ...:
        ...
    @yyzy.setter
    def yyzy(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yyzz(self) -> ...:
        ...
    @yyzz.setter
    def yyzz(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yzxx(self) -> ...:
        ...
    @yzxx.setter
    def yzxx(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yzxy(self) -> ...:
        ...
    @yzxy.setter
    def yzxy(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yzxz(self) -> ...:
        ...
    @yzxz.setter
    def yzxz(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yzyx(self) -> ...:
        ...
    @yzyx.setter
    def yzyx(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yzyy(self) -> ...:
        ...
    @yzyy.setter
    def yzyy(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yzyz(self) -> ...:
        ...
    @yzyz.setter
    def yzyz(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yzzx(self) -> ...:
        ...
    @yzzx.setter
    def yzzx(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yzzy(self) -> ...:
        ...
    @yzzy.setter
    def yzzy(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def yzzz(self) -> ...:
        ...
    @yzzz.setter
    def yzzz(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def zxxx(self) -> ...:
        ...
    @zxxx.setter
    def zxxx(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def zxxy(self) -> ...:
        ...
    @zxxy.setter
    def zxxy(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def zxxz(self) -> ...:
        ...
    @zxxz.setter
    def zxxz(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def zxyx(self) -> ...:
        ...
    @zxyx.setter
    def zxyx(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def zxyy(self) -> ...:
        ...
    @zxyy.setter
    def zxyy(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def zxyz(self) -> ...:
        ...
    @zxyz.setter
    def zxyz(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def zxzx(self) -> ...:
        ...
    @zxzx.setter
    def zxzx(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def zxzy(self) -> ...:
        ...
    @zxzy.setter
    def zxzy(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def zxzz(self) -> ...:
        ...
    @zxzz.setter
    def zxzz(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def zyxx(self) -> ...:
        ...
    @zyxx.setter
    def zyxx(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def zyxy(self) -> ...:
        ...
    @zyxy.setter
    def zyxy(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def zyxz(self) -> ...:
        ...
    @zyxz.setter
    def zyxz(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def zyyx(self) -> ...:
        ...
    @zyyx.setter
    def zyyx(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def zyyy(self) -> ...:
        ...
    @zyyy.setter
    def zyyy(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def zyyz(self) -> ...:
        ...
    @zyyz.setter
    def zyyz(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def zyzx(self) -> ...:
        ...
    @zyzx.setter
    def zyzx(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def zyzy(self) -> ...:
        ...
    @zyzy.setter
    def zyzy(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def zyzz(self) -> ...:
        ...
    @zyzz.setter
    def zyzz(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def zzxx(self) -> ...:
        ...
    @zzxx.setter
    def zzxx(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def zzxy(self) -> ...:
        ...
    @zzxy.setter
    def zzxy(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def zzxz(self) -> ...:
        ...
    @zzxz.setter
    def zzxz(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def zzyx(self) -> ...:
        ...
    @zzyx.setter
    def zzyx(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def zzyy(self) -> ...:
        ...
    @zzyy.setter
    def zzyy(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def zzyz(self) -> ...:
        ...
    @zzyz.setter
    def zzyz(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def zzzx(self) -> ...:
        ...
    @zzzx.setter
    def zzzx(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def zzzy(self) -> ...:
        ...
    @zzzy.setter
    def zzzy(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
    @property
    def zzzz(self) -> ...:
        ...
    @zzzz.setter
    def zzzz(*args, **kwargs):
        """
        (arg0: ocapi.float3, arg1: ocarina::Vector<float,4>) -> None
        """
class float3x2:
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    def __add__(self, arg0: float3x2) -> float3x2:
        ...
    def __getitem__(self, arg0: int) -> float2:
        ...
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: float) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: float3x2) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(6)]) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[float2], pybind11_stubgen.typing_ext.FixedSize(3)]) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(2)]], pybind11_stubgen.typing_ext.FixedSize(3)]) -> None:
        ...
    @typing.overload
    def __mul__(self, arg0: float) -> float3x2:
        ...
    @typing.overload
    def __mul__(self, arg0: float3) -> float2:
        ...
    @typing.overload
    def __mul__(self, arg0: float2x3) -> float2x2:
        ...
    def __neg__(self) -> float3x2:
        ...
    def __repr__(self) -> str:
        ...
    def __rmul__(self, arg0: float) -> float3x2:
        ...
    def __setitem__(self, arg0: int, arg1: float2) -> None:
        ...
    def __sub__(self, arg0: float3x2) -> float3x2:
        ...
    def __truediv__(self, arg0: float) -> float3x2:
        ...
    @typing.overload
    def clone(self) -> float3x2:
        ...
    @typing.overload
    def clone(self) -> float3x2:
        ...
    @property
    def desc_(self) -> str:
        ...
class float3x3:
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    def __add__(self, arg0: float3x3) -> float3x3:
        ...
    def __getitem__(self, arg0: int) -> float3:
        ...
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: float) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: float3x3) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(9)]) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[float3], pybind11_stubgen.typing_ext.FixedSize(3)]) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(3)]], pybind11_stubgen.typing_ext.FixedSize(3)]) -> None:
        ...
    @typing.overload
    def __mul__(self, arg0: float) -> float3x3:
        ...
    @typing.overload
    def __mul__(self, arg0: float3) -> float3:
        ...
    @typing.overload
    def __mul__(self, arg0: float3x3) -> float3x3:
        ...
    def __neg__(self) -> float3x3:
        ...
    def __repr__(self) -> str:
        ...
    def __rmul__(self, arg0: float) -> float3x3:
        ...
    def __setitem__(self, arg0: int, arg1: float3) -> None:
        ...
    def __sub__(self, arg0: float3x3) -> float3x3:
        ...
    def __truediv__(self, arg0: float) -> float3x3:
        ...
    @typing.overload
    def clone(self) -> float3x3:
        ...
    @typing.overload
    def clone(self) -> float3x3:
        ...
    @property
    def desc_(self) -> str:
        ...
class float3x4:
    @staticmethod
    @typing.overload
    def __mul__(*args, **kwargs) -> ...:
        ...
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    def __add__(self, arg0: float3x4) -> float3x4:
        ...
    def __getitem__(self, arg0: int) -> float4:
        ...
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: float) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: float3x4) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(12)]) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[float4], pybind11_stubgen.typing_ext.FixedSize(3)]) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(4)]], pybind11_stubgen.typing_ext.FixedSize(3)]) -> None:
        ...
    @typing.overload
    def __mul__(self, arg0: float) -> float3x4:
        ...
    @typing.overload
    def __mul__(self, arg0: float3) -> float4:
        ...
    def __neg__(self) -> float3x4:
        ...
    def __repr__(self) -> str:
        ...
    def __rmul__(self, arg0: float) -> float3x4:
        ...
    def __setitem__(self, arg0: int, arg1: float4) -> None:
        ...
    def __sub__(self, arg0: float3x4) -> float3x4:
        ...
    def __truediv__(self, arg0: float) -> float3x4:
        ...
    @typing.overload
    def clone(self) -> float3x4:
        ...
    @typing.overload
    def clone(self) -> float3x4:
        ...
    @property
    def desc_(self) -> str:
        ...
class float4(_VectorStoragefloat4):
    __hash__: typing.ClassVar[None] = None
    w: float
    ww: float2
    www: float3
    wwww: float4
    wwwx: float4
    wwwy: float4
    wwwz: float4
    wwx: float3
    wwxw: float4
    wwxx: float4
    wwxy: float4
    wwxz: float4
    wwy: float3
    wwyw: float4
    wwyx: float4
    wwyy: float4
    wwyz: float4
    wwz: float3
    wwzw: float4
    wwzx: float4
    wwzy: float4
    wwzz: float4
    wx: float2
    wxw: float3
    wxww: float4
    wxwx: float4
    wxwy: float4
    wxwz: float4
    wxx: float3
    wxxw: float4
    wxxx: float4
    wxxy: float4
    wxxz: float4
    wxy: float3
    wxyw: float4
    wxyx: float4
    wxyy: float4
    wxyz: float4
    wxz: float3
    wxzw: float4
    wxzx: float4
    wxzy: float4
    wxzz: float4
    wy: float2
    wyw: float3
    wyww: float4
    wywx: float4
    wywy: float4
    wywz: float4
    wyx: float3
    wyxw: float4
    wyxx: float4
    wyxy: float4
    wyxz: float4
    wyy: float3
    wyyw: float4
    wyyx: float4
    wyyy: float4
    wyyz: float4
    wyz: float3
    wyzw: float4
    wyzx: float4
    wyzy: float4
    wyzz: float4
    wz: float2
    wzw: float3
    wzww: float4
    wzwx: float4
    wzwy: float4
    wzwz: float4
    wzx: float3
    wzxw: float4
    wzxx: float4
    wzxy: float4
    wzxz: float4
    wzy: float3
    wzyw: float4
    wzyx: float4
    wzyy: float4
    wzyz: float4
    wzz: float3
    wzzw: float4
    wzzx: float4
    wzzy: float4
    wzzz: float4
    x: float
    xw: float2
    xww: float3
    xwww: float4
    xwwx: float4
    xwwy: float4
    xwwz: float4
    xwx: float3
    xwxw: float4
    xwxx: float4
    xwxy: float4
    xwxz: float4
    xwy: float3
    xwyw: float4
    xwyx: float4
    xwyy: float4
    xwyz: float4
    xwz: float3
    xwzw: float4
    xwzx: float4
    xwzy: float4
    xwzz: float4
    xx: float2
    xxw: float3
    xxww: float4
    xxwx: float4
    xxwy: float4
    xxwz: float4
    xxx: float3
    xxxw: float4
    xxxx: float4
    xxxy: float4
    xxxz: float4
    xxy: float3
    xxyw: float4
    xxyx: float4
    xxyy: float4
    xxyz: float4
    xxz: float3
    xxzw: float4
    xxzx: float4
    xxzy: float4
    xxzz: float4
    xy: float2
    xyw: float3
    xyww: float4
    xywx: float4
    xywy: float4
    xywz: float4
    xyx: float3
    xyxw: float4
    xyxx: float4
    xyxy: float4
    xyxz: float4
    xyy: float3
    xyyw: float4
    xyyx: float4
    xyyy: float4
    xyyz: float4
    xyz: float3
    xyzw: float4
    xyzx: float4
    xyzy: float4
    xyzz: float4
    xz: float2
    xzw: float3
    xzww: float4
    xzwx: float4
    xzwy: float4
    xzwz: float4
    xzx: float3
    xzxw: float4
    xzxx: float4
    xzxy: float4
    xzxz: float4
    xzy: float3
    xzyw: float4
    xzyx: float4
    xzyy: float4
    xzyz: float4
    xzz: float3
    xzzw: float4
    xzzx: float4
    xzzy: float4
    xzzz: float4
    y: float
    yw: float2
    yww: float3
    ywww: float4
    ywwx: float4
    ywwy: float4
    ywwz: float4
    ywx: float3
    ywxw: float4
    ywxx: float4
    ywxy: float4
    ywxz: float4
    ywy: float3
    ywyw: float4
    ywyx: float4
    ywyy: float4
    ywyz: float4
    ywz: float3
    ywzw: float4
    ywzx: float4
    ywzy: float4
    ywzz: float4
    yx: float2
    yxw: float3
    yxww: float4
    yxwx: float4
    yxwy: float4
    yxwz: float4
    yxx: float3
    yxxw: float4
    yxxx: float4
    yxxy: float4
    yxxz: float4
    yxy: float3
    yxyw: float4
    yxyx: float4
    yxyy: float4
    yxyz: float4
    yxz: float3
    yxzw: float4
    yxzx: float4
    yxzy: float4
    yxzz: float4
    yy: float2
    yyw: float3
    yyww: float4
    yywx: float4
    yywy: float4
    yywz: float4
    yyx: float3
    yyxw: float4
    yyxx: float4
    yyxy: float4
    yyxz: float4
    yyy: float3
    yyyw: float4
    yyyx: float4
    yyyy: float4
    yyyz: float4
    yyz: float3
    yyzw: float4
    yyzx: float4
    yyzy: float4
    yyzz: float4
    yz: float2
    yzw: float3
    yzww: float4
    yzwx: float4
    yzwy: float4
    yzwz: float4
    yzx: float3
    yzxw: float4
    yzxx: float4
    yzxy: float4
    yzxz: float4
    yzy: float3
    yzyw: float4
    yzyx: float4
    yzyy: float4
    yzyz: float4
    yzz: float3
    yzzw: float4
    yzzx: float4
    yzzy: float4
    yzzz: float4
    z: float
    zw: float2
    zww: float3
    zwww: float4
    zwwx: float4
    zwwy: float4
    zwwz: float4
    zwx: float3
    zwxw: float4
    zwxx: float4
    zwxy: float4
    zwxz: float4
    zwy: float3
    zwyw: float4
    zwyx: float4
    zwyy: float4
    zwyz: float4
    zwz: float3
    zwzw: float4
    zwzx: float4
    zwzy: float4
    zwzz: float4
    zx: float2
    zxw: float3
    zxww: float4
    zxwx: float4
    zxwy: float4
    zxwz: float4
    zxx: float3
    zxxw: float4
    zxxx: float4
    zxxy: float4
    zxxz: float4
    zxy: float3
    zxyw: float4
    zxyx: float4
    zxyy: float4
    zxyz: float4
    zxz: float3
    zxzw: float4
    zxzx: float4
    zxzy: float4
    zxzz: float4
    zy: float2
    zyw: float3
    zyww: float4
    zywx: float4
    zywy: float4
    zywz: float4
    zyx: float3
    zyxw: float4
    zyxx: float4
    zyxy: float4
    zyxz: float4
    zyy: float3
    zyyw: float4
    zyyx: float4
    zyyy: float4
    zyyz: float4
    zyz: float3
    zyzw: float4
    zyzx: float4
    zyzy: float4
    zyzz: float4
    zz: float2
    zzw: float3
    zzww: float4
    zzwx: float4
    zzwy: float4
    zzwz: float4
    zzx: float3
    zzxw: float4
    zzxx: float4
    zzxy: float4
    zzxz: float4
    zzy: float3
    zzyw: float4
    zzyx: float4
    zzyy: float4
    zzyz: float4
    zzz: float3
    zzzw: float4
    zzzx: float4
    zzzy: float4
    zzzz: float4
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    @typing.overload
    def __add__(self, arg0: float4) -> float4:
        ...
    @typing.overload
    def __add__(self, arg0: float) -> float4:
        ...
    def __eq__(self, arg0: float4) -> ...:
        ...
    def __ge__(self, arg0: float4) -> ...:
        ...
    def __getitem__(self, arg0: int) -> float:
        ...
    def __gt__(self, arg0: float4) -> ...:
        ...
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: float) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: float, arg1: float, arg2: float, arg3: float) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(4)]) -> None:
        ...
    def __le__(self, arg0: float4) -> ...:
        ...
    def __lt__(self, arg0: float4) -> ...:
        ...
    @typing.overload
    def __mul__(self, arg0: float4) -> float4:
        ...
    @typing.overload
    def __mul__(self, arg0: float) -> float4:
        ...
    def __ne__(self, arg0: float4) -> ...:
        ...
    def __neg__(self) -> float4:
        ...
    def __radd__(self, arg0: float) -> float4:
        ...
    def __repr__(self) -> str:
        ...
    def __rmul__(self, arg0: float) -> float4:
        ...
    def __rsub__(self, arg0: float) -> float4:
        ...
    def __rtruediv__(self, arg0: float) -> float4:
        ...
    def __setitem__(self, arg0: int, arg1: float) -> None:
        ...
    @typing.overload
    def __sub__(self, arg0: float4) -> float4:
        ...
    @typing.overload
    def __sub__(self, arg0: float) -> float4:
        ...
    @typing.overload
    def __truediv__(self, arg0: float4) -> float4:
        ...
    @typing.overload
    def __truediv__(self, arg0: float) -> float4:
        ...
    def clone(self) -> float4:
        ...
    @property
    def desc_(self) -> str:
        ...
class float4x2:
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    def __add__(self, arg0: float4x2) -> float4x2:
        ...
    def __getitem__(self, arg0: int) -> float2:
        ...
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: float) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: float4x2) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(8)]) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[float2], pybind11_stubgen.typing_ext.FixedSize(4)]) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(2)]], pybind11_stubgen.typing_ext.FixedSize(4)]) -> None:
        ...
    @typing.overload
    def __mul__(self, arg0: float) -> float4x2:
        ...
    @typing.overload
    def __mul__(self, arg0: float4) -> float2:
        ...
    @typing.overload
    def __mul__(self, arg0: float2x4) -> float2x2:
        ...
    def __neg__(self) -> float4x2:
        ...
    def __repr__(self) -> str:
        ...
    def __rmul__(self, arg0: float) -> float4x2:
        ...
    def __setitem__(self, arg0: int, arg1: float2) -> None:
        ...
    def __sub__(self, arg0: float4x2) -> float4x2:
        ...
    def __truediv__(self, arg0: float) -> float4x2:
        ...
    @typing.overload
    def clone(self) -> float4x2:
        ...
    @typing.overload
    def clone(self) -> float4x2:
        ...
    @property
    def desc_(self) -> str:
        ...
class float4x3:
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    def __add__(self, arg0: float4x3) -> float4x3:
        ...
    def __getitem__(self, arg0: int) -> float3:
        ...
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: float) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: float4x3) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(12)]) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[float3], pybind11_stubgen.typing_ext.FixedSize(4)]) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(3)]], pybind11_stubgen.typing_ext.FixedSize(4)]) -> None:
        ...
    @typing.overload
    def __mul__(self, arg0: float) -> float4x3:
        ...
    @typing.overload
    def __mul__(self, arg0: float4) -> float3:
        ...
    @typing.overload
    def __mul__(self, arg0: float3x4) -> float3x3:
        ...
    def __neg__(self) -> float4x3:
        ...
    def __repr__(self) -> str:
        ...
    def __rmul__(self, arg0: float) -> float4x3:
        ...
    def __setitem__(self, arg0: int, arg1: float3) -> None:
        ...
    def __sub__(self, arg0: float4x3) -> float4x3:
        ...
    def __truediv__(self, arg0: float) -> float4x3:
        ...
    @typing.overload
    def clone(self) -> float4x3:
        ...
    @typing.overload
    def clone(self) -> float4x3:
        ...
    @property
    def desc_(self) -> str:
        ...
class float4x4:
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    def __add__(self, arg0: float4x4) -> float4x4:
        ...
    def __getitem__(self, arg0: int) -> float4:
        ...
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: float) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: float4x4) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(16)]) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[float4], pybind11_stubgen.typing_ext.FixedSize(4)]) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(4)]], pybind11_stubgen.typing_ext.FixedSize(4)]) -> None:
        ...
    @typing.overload
    def __mul__(self, arg0: float) -> float4x4:
        ...
    @typing.overload
    def __mul__(self, arg0: float4) -> float4:
        ...
    @typing.overload
    def __mul__(self, arg0: float4x4) -> float4x4:
        ...
    def __neg__(self) -> float4x4:
        ...
    def __repr__(self) -> str:
        ...
    def __rmul__(self, arg0: float) -> float4x4:
        ...
    def __setitem__(self, arg0: int, arg1: float4) -> None:
        ...
    def __sub__(self, arg0: float4x4) -> float4x4:
        ...
    def __truediv__(self, arg0: float) -> float4x4:
        ...
    @typing.overload
    def clone(self) -> float4x4:
        ...
    @typing.overload
    def clone(self) -> float4x4:
        ...
    @property
    def desc_(self) -> str:
        ...
class int2(_VectorStorageint2):
    __hash__: typing.ClassVar[None] = None
    x: int
    xx: int2
    xy: int2
    y: int
    yx: int2
    yy: int2
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    @typing.overload
    def __add__(self, arg0: int2) -> int2:
        ...
    @typing.overload
    def __add__(self, arg0: int) -> int2:
        ...
    def __and__(self, arg0: int2) -> int2:
        ...
    def __eq__(self, arg0: int2) -> ...:
        ...
    def __ge__(self, arg0: int2) -> ...:
        ...
    def __getitem__(self, arg0: int) -> int:
        ...
    def __gt__(self, arg0: int2) -> ...:
        ...
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: int) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: int, arg1: int) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[int], pybind11_stubgen.typing_ext.FixedSize(2)]) -> None:
        ...
    def __le__(self, arg0: int2) -> ...:
        ...
    def __lt__(self, arg0: int2) -> ...:
        ...
    @typing.overload
    def __mod__(self, arg0: int2) -> int2:
        ...
    @typing.overload
    def __mod__(self, arg0: int) -> int2:
        ...
    @typing.overload
    def __mul__(self, arg0: int2) -> int2:
        ...
    @typing.overload
    def __mul__(self, arg0: int) -> int2:
        ...
    def __ne__(self, arg0: int2) -> ...:
        ...
    def __neg__(self) -> int2:
        ...
    def __or__(self, arg0: int2) -> int2:
        ...
    def __radd__(self, arg0: int) -> int2:
        ...
    def __repr__(self) -> str:
        ...
    def __rmod__(self, arg0: int) -> int2:
        ...
    def __rmul__(self, arg0: int) -> int2:
        ...
    def __rsub__(self, arg0: int) -> int2:
        ...
    def __rtruediv__(self, arg0: int) -> int2:
        ...
    def __setitem__(self, arg0: int, arg1: int) -> None:
        ...
    def __shl__(self, arg0: int) -> int2:
        ...
    def __shr__(self, arg0: int) -> int2:
        ...
    @typing.overload
    def __sub__(self, arg0: int2) -> int2:
        ...
    @typing.overload
    def __sub__(self, arg0: int) -> int2:
        ...
    @typing.overload
    def __truediv__(self, arg0: int2) -> int2:
        ...
    @typing.overload
    def __truediv__(self, arg0: int) -> int2:
        ...
    def __xor__(self, arg0: int2) -> int2:
        ...
    def clone(self) -> int2:
        ...
    @property
    def desc_(self) -> str:
        ...
    @property
    def xxx(self) -> ...:
        ...
    @xxx.setter
    def xxx(*args, **kwargs):
        """
        (arg0: ocapi.int2, arg1: ocarina::Vector<int,3>) -> None
        """
    @property
    def xxxx(self) -> ...:
        ...
    @xxxx.setter
    def xxxx(*args, **kwargs):
        """
        (arg0: ocapi.int2, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def xxxy(self) -> ...:
        ...
    @xxxy.setter
    def xxxy(*args, **kwargs):
        """
        (arg0: ocapi.int2, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def xxy(self) -> ...:
        ...
    @xxy.setter
    def xxy(*args, **kwargs):
        """
        (arg0: ocapi.int2, arg1: ocarina::Vector<int,3>) -> None
        """
    @property
    def xxyx(self) -> ...:
        ...
    @xxyx.setter
    def xxyx(*args, **kwargs):
        """
        (arg0: ocapi.int2, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def xxyy(self) -> ...:
        ...
    @xxyy.setter
    def xxyy(*args, **kwargs):
        """
        (arg0: ocapi.int2, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def xyx(self) -> ...:
        ...
    @xyx.setter
    def xyx(*args, **kwargs):
        """
        (arg0: ocapi.int2, arg1: ocarina::Vector<int,3>) -> None
        """
    @property
    def xyxx(self) -> ...:
        ...
    @xyxx.setter
    def xyxx(*args, **kwargs):
        """
        (arg0: ocapi.int2, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def xyxy(self) -> ...:
        ...
    @xyxy.setter
    def xyxy(*args, **kwargs):
        """
        (arg0: ocapi.int2, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def xyy(self) -> ...:
        ...
    @xyy.setter
    def xyy(*args, **kwargs):
        """
        (arg0: ocapi.int2, arg1: ocarina::Vector<int,3>) -> None
        """
    @property
    def xyyx(self) -> ...:
        ...
    @xyyx.setter
    def xyyx(*args, **kwargs):
        """
        (arg0: ocapi.int2, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def xyyy(self) -> ...:
        ...
    @xyyy.setter
    def xyyy(*args, **kwargs):
        """
        (arg0: ocapi.int2, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yxx(self) -> ...:
        ...
    @yxx.setter
    def yxx(*args, **kwargs):
        """
        (arg0: ocapi.int2, arg1: ocarina::Vector<int,3>) -> None
        """
    @property
    def yxxx(self) -> ...:
        ...
    @yxxx.setter
    def yxxx(*args, **kwargs):
        """
        (arg0: ocapi.int2, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yxxy(self) -> ...:
        ...
    @yxxy.setter
    def yxxy(*args, **kwargs):
        """
        (arg0: ocapi.int2, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yxy(self) -> ...:
        ...
    @yxy.setter
    def yxy(*args, **kwargs):
        """
        (arg0: ocapi.int2, arg1: ocarina::Vector<int,3>) -> None
        """
    @property
    def yxyx(self) -> ...:
        ...
    @yxyx.setter
    def yxyx(*args, **kwargs):
        """
        (arg0: ocapi.int2, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yxyy(self) -> ...:
        ...
    @yxyy.setter
    def yxyy(*args, **kwargs):
        """
        (arg0: ocapi.int2, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yyx(self) -> ...:
        ...
    @yyx.setter
    def yyx(*args, **kwargs):
        """
        (arg0: ocapi.int2, arg1: ocarina::Vector<int,3>) -> None
        """
    @property
    def yyxx(self) -> ...:
        ...
    @yyxx.setter
    def yyxx(*args, **kwargs):
        """
        (arg0: ocapi.int2, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yyxy(self) -> ...:
        ...
    @yyxy.setter
    def yyxy(*args, **kwargs):
        """
        (arg0: ocapi.int2, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yyy(self) -> ...:
        ...
    @yyy.setter
    def yyy(*args, **kwargs):
        """
        (arg0: ocapi.int2, arg1: ocarina::Vector<int,3>) -> None
        """
    @property
    def yyyx(self) -> ...:
        ...
    @yyyx.setter
    def yyyx(*args, **kwargs):
        """
        (arg0: ocapi.int2, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yyyy(self) -> ...:
        ...
    @yyyy.setter
    def yyyy(*args, **kwargs):
        """
        (arg0: ocapi.int2, arg1: ocarina::Vector<int,4>) -> None
        """
class int3(_VectorStorageint3):
    __hash__: typing.ClassVar[None] = None
    x: int
    xx: int2
    xxx: int3
    xxy: int3
    xxz: int3
    xy: int2
    xyx: int3
    xyy: int3
    xyz: int3
    xz: int2
    xzx: int3
    xzy: int3
    xzz: int3
    y: int
    yx: int2
    yxx: int3
    yxy: int3
    yxz: int3
    yy: int2
    yyx: int3
    yyy: int3
    yyz: int3
    yz: int2
    yzx: int3
    yzy: int3
    yzz: int3
    z: int
    zx: int2
    zxx: int3
    zxy: int3
    zxz: int3
    zy: int2
    zyx: int3
    zyy: int3
    zyz: int3
    zz: int2
    zzx: int3
    zzy: int3
    zzz: int3
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    @typing.overload
    def __add__(self, arg0: int3) -> int3:
        ...
    @typing.overload
    def __add__(self, arg0: int) -> int3:
        ...
    def __and__(self, arg0: int3) -> int3:
        ...
    def __eq__(self, arg0: int3) -> ...:
        ...
    def __ge__(self, arg0: int3) -> ...:
        ...
    def __getitem__(self, arg0: int) -> int:
        ...
    def __gt__(self, arg0: int3) -> ...:
        ...
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: int) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: int, arg1: int, arg2: int) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[int], pybind11_stubgen.typing_ext.FixedSize(3)]) -> None:
        ...
    def __le__(self, arg0: int3) -> ...:
        ...
    def __lt__(self, arg0: int3) -> ...:
        ...
    @typing.overload
    def __mod__(self, arg0: int3) -> int3:
        ...
    @typing.overload
    def __mod__(self, arg0: int) -> int3:
        ...
    @typing.overload
    def __mul__(self, arg0: int3) -> int3:
        ...
    @typing.overload
    def __mul__(self, arg0: int) -> int3:
        ...
    def __ne__(self, arg0: int3) -> ...:
        ...
    def __neg__(self) -> int3:
        ...
    def __or__(self, arg0: int3) -> int3:
        ...
    def __radd__(self, arg0: int) -> int3:
        ...
    def __repr__(self) -> str:
        ...
    def __rmod__(self, arg0: int) -> int3:
        ...
    def __rmul__(self, arg0: int) -> int3:
        ...
    def __rsub__(self, arg0: int) -> int3:
        ...
    def __rtruediv__(self, arg0: int) -> int3:
        ...
    def __setitem__(self, arg0: int, arg1: int) -> None:
        ...
    def __shl__(self, arg0: int) -> int3:
        ...
    def __shr__(self, arg0: int) -> int3:
        ...
    @typing.overload
    def __sub__(self, arg0: int3) -> int3:
        ...
    @typing.overload
    def __sub__(self, arg0: int) -> int3:
        ...
    @typing.overload
    def __truediv__(self, arg0: int3) -> int3:
        ...
    @typing.overload
    def __truediv__(self, arg0: int) -> int3:
        ...
    def __xor__(self, arg0: int3) -> int3:
        ...
    def clone(self) -> int3:
        ...
    @property
    def desc_(self) -> str:
        ...
    @property
    def xxxx(self) -> ...:
        ...
    @xxxx.setter
    def xxxx(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def xxxy(self) -> ...:
        ...
    @xxxy.setter
    def xxxy(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def xxxz(self) -> ...:
        ...
    @xxxz.setter
    def xxxz(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def xxyx(self) -> ...:
        ...
    @xxyx.setter
    def xxyx(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def xxyy(self) -> ...:
        ...
    @xxyy.setter
    def xxyy(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def xxyz(self) -> ...:
        ...
    @xxyz.setter
    def xxyz(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def xxzx(self) -> ...:
        ...
    @xxzx.setter
    def xxzx(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def xxzy(self) -> ...:
        ...
    @xxzy.setter
    def xxzy(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def xxzz(self) -> ...:
        ...
    @xxzz.setter
    def xxzz(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def xyxx(self) -> ...:
        ...
    @xyxx.setter
    def xyxx(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def xyxy(self) -> ...:
        ...
    @xyxy.setter
    def xyxy(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def xyxz(self) -> ...:
        ...
    @xyxz.setter
    def xyxz(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def xyyx(self) -> ...:
        ...
    @xyyx.setter
    def xyyx(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def xyyy(self) -> ...:
        ...
    @xyyy.setter
    def xyyy(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def xyyz(self) -> ...:
        ...
    @xyyz.setter
    def xyyz(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def xyzx(self) -> ...:
        ...
    @xyzx.setter
    def xyzx(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def xyzy(self) -> ...:
        ...
    @xyzy.setter
    def xyzy(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def xyzz(self) -> ...:
        ...
    @xyzz.setter
    def xyzz(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def xzxx(self) -> ...:
        ...
    @xzxx.setter
    def xzxx(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def xzxy(self) -> ...:
        ...
    @xzxy.setter
    def xzxy(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def xzxz(self) -> ...:
        ...
    @xzxz.setter
    def xzxz(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def xzyx(self) -> ...:
        ...
    @xzyx.setter
    def xzyx(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def xzyy(self) -> ...:
        ...
    @xzyy.setter
    def xzyy(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def xzyz(self) -> ...:
        ...
    @xzyz.setter
    def xzyz(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def xzzx(self) -> ...:
        ...
    @xzzx.setter
    def xzzx(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def xzzy(self) -> ...:
        ...
    @xzzy.setter
    def xzzy(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def xzzz(self) -> ...:
        ...
    @xzzz.setter
    def xzzz(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yxxx(self) -> ...:
        ...
    @yxxx.setter
    def yxxx(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yxxy(self) -> ...:
        ...
    @yxxy.setter
    def yxxy(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yxxz(self) -> ...:
        ...
    @yxxz.setter
    def yxxz(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yxyx(self) -> ...:
        ...
    @yxyx.setter
    def yxyx(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yxyy(self) -> ...:
        ...
    @yxyy.setter
    def yxyy(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yxyz(self) -> ...:
        ...
    @yxyz.setter
    def yxyz(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yxzx(self) -> ...:
        ...
    @yxzx.setter
    def yxzx(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yxzy(self) -> ...:
        ...
    @yxzy.setter
    def yxzy(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yxzz(self) -> ...:
        ...
    @yxzz.setter
    def yxzz(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yyxx(self) -> ...:
        ...
    @yyxx.setter
    def yyxx(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yyxy(self) -> ...:
        ...
    @yyxy.setter
    def yyxy(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yyxz(self) -> ...:
        ...
    @yyxz.setter
    def yyxz(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yyyx(self) -> ...:
        ...
    @yyyx.setter
    def yyyx(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yyyy(self) -> ...:
        ...
    @yyyy.setter
    def yyyy(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yyyz(self) -> ...:
        ...
    @yyyz.setter
    def yyyz(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yyzx(self) -> ...:
        ...
    @yyzx.setter
    def yyzx(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yyzy(self) -> ...:
        ...
    @yyzy.setter
    def yyzy(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yyzz(self) -> ...:
        ...
    @yyzz.setter
    def yyzz(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yzxx(self) -> ...:
        ...
    @yzxx.setter
    def yzxx(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yzxy(self) -> ...:
        ...
    @yzxy.setter
    def yzxy(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yzxz(self) -> ...:
        ...
    @yzxz.setter
    def yzxz(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yzyx(self) -> ...:
        ...
    @yzyx.setter
    def yzyx(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yzyy(self) -> ...:
        ...
    @yzyy.setter
    def yzyy(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yzyz(self) -> ...:
        ...
    @yzyz.setter
    def yzyz(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yzzx(self) -> ...:
        ...
    @yzzx.setter
    def yzzx(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yzzy(self) -> ...:
        ...
    @yzzy.setter
    def yzzy(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def yzzz(self) -> ...:
        ...
    @yzzz.setter
    def yzzz(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def zxxx(self) -> ...:
        ...
    @zxxx.setter
    def zxxx(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def zxxy(self) -> ...:
        ...
    @zxxy.setter
    def zxxy(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def zxxz(self) -> ...:
        ...
    @zxxz.setter
    def zxxz(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def zxyx(self) -> ...:
        ...
    @zxyx.setter
    def zxyx(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def zxyy(self) -> ...:
        ...
    @zxyy.setter
    def zxyy(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def zxyz(self) -> ...:
        ...
    @zxyz.setter
    def zxyz(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def zxzx(self) -> ...:
        ...
    @zxzx.setter
    def zxzx(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def zxzy(self) -> ...:
        ...
    @zxzy.setter
    def zxzy(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def zxzz(self) -> ...:
        ...
    @zxzz.setter
    def zxzz(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def zyxx(self) -> ...:
        ...
    @zyxx.setter
    def zyxx(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def zyxy(self) -> ...:
        ...
    @zyxy.setter
    def zyxy(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def zyxz(self) -> ...:
        ...
    @zyxz.setter
    def zyxz(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def zyyx(self) -> ...:
        ...
    @zyyx.setter
    def zyyx(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def zyyy(self) -> ...:
        ...
    @zyyy.setter
    def zyyy(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def zyyz(self) -> ...:
        ...
    @zyyz.setter
    def zyyz(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def zyzx(self) -> ...:
        ...
    @zyzx.setter
    def zyzx(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def zyzy(self) -> ...:
        ...
    @zyzy.setter
    def zyzy(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def zyzz(self) -> ...:
        ...
    @zyzz.setter
    def zyzz(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def zzxx(self) -> ...:
        ...
    @zzxx.setter
    def zzxx(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def zzxy(self) -> ...:
        ...
    @zzxy.setter
    def zzxy(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def zzxz(self) -> ...:
        ...
    @zzxz.setter
    def zzxz(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def zzyx(self) -> ...:
        ...
    @zzyx.setter
    def zzyx(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def zzyy(self) -> ...:
        ...
    @zzyy.setter
    def zzyy(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def zzyz(self) -> ...:
        ...
    @zzyz.setter
    def zzyz(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def zzzx(self) -> ...:
        ...
    @zzzx.setter
    def zzzx(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def zzzy(self) -> ...:
        ...
    @zzzy.setter
    def zzzy(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
    @property
    def zzzz(self) -> ...:
        ...
    @zzzz.setter
    def zzzz(*args, **kwargs):
        """
        (arg0: ocapi.int3, arg1: ocarina::Vector<int,4>) -> None
        """
class int4(_VectorStorageint4):
    __hash__: typing.ClassVar[None] = None
    w: int
    ww: int2
    www: int3
    wwww: int4
    wwwx: int4
    wwwy: int4
    wwwz: int4
    wwx: int3
    wwxw: int4
    wwxx: int4
    wwxy: int4
    wwxz: int4
    wwy: int3
    wwyw: int4
    wwyx: int4
    wwyy: int4
    wwyz: int4
    wwz: int3
    wwzw: int4
    wwzx: int4
    wwzy: int4
    wwzz: int4
    wx: int2
    wxw: int3
    wxww: int4
    wxwx: int4
    wxwy: int4
    wxwz: int4
    wxx: int3
    wxxw: int4
    wxxx: int4
    wxxy: int4
    wxxz: int4
    wxy: int3
    wxyw: int4
    wxyx: int4
    wxyy: int4
    wxyz: int4
    wxz: int3
    wxzw: int4
    wxzx: int4
    wxzy: int4
    wxzz: int4
    wy: int2
    wyw: int3
    wyww: int4
    wywx: int4
    wywy: int4
    wywz: int4
    wyx: int3
    wyxw: int4
    wyxx: int4
    wyxy: int4
    wyxz: int4
    wyy: int3
    wyyw: int4
    wyyx: int4
    wyyy: int4
    wyyz: int4
    wyz: int3
    wyzw: int4
    wyzx: int4
    wyzy: int4
    wyzz: int4
    wz: int2
    wzw: int3
    wzww: int4
    wzwx: int4
    wzwy: int4
    wzwz: int4
    wzx: int3
    wzxw: int4
    wzxx: int4
    wzxy: int4
    wzxz: int4
    wzy: int3
    wzyw: int4
    wzyx: int4
    wzyy: int4
    wzyz: int4
    wzz: int3
    wzzw: int4
    wzzx: int4
    wzzy: int4
    wzzz: int4
    x: int
    xw: int2
    xww: int3
    xwww: int4
    xwwx: int4
    xwwy: int4
    xwwz: int4
    xwx: int3
    xwxw: int4
    xwxx: int4
    xwxy: int4
    xwxz: int4
    xwy: int3
    xwyw: int4
    xwyx: int4
    xwyy: int4
    xwyz: int4
    xwz: int3
    xwzw: int4
    xwzx: int4
    xwzy: int4
    xwzz: int4
    xx: int2
    xxw: int3
    xxww: int4
    xxwx: int4
    xxwy: int4
    xxwz: int4
    xxx: int3
    xxxw: int4
    xxxx: int4
    xxxy: int4
    xxxz: int4
    xxy: int3
    xxyw: int4
    xxyx: int4
    xxyy: int4
    xxyz: int4
    xxz: int3
    xxzw: int4
    xxzx: int4
    xxzy: int4
    xxzz: int4
    xy: int2
    xyw: int3
    xyww: int4
    xywx: int4
    xywy: int4
    xywz: int4
    xyx: int3
    xyxw: int4
    xyxx: int4
    xyxy: int4
    xyxz: int4
    xyy: int3
    xyyw: int4
    xyyx: int4
    xyyy: int4
    xyyz: int4
    xyz: int3
    xyzw: int4
    xyzx: int4
    xyzy: int4
    xyzz: int4
    xz: int2
    xzw: int3
    xzww: int4
    xzwx: int4
    xzwy: int4
    xzwz: int4
    xzx: int3
    xzxw: int4
    xzxx: int4
    xzxy: int4
    xzxz: int4
    xzy: int3
    xzyw: int4
    xzyx: int4
    xzyy: int4
    xzyz: int4
    xzz: int3
    xzzw: int4
    xzzx: int4
    xzzy: int4
    xzzz: int4
    y: int
    yw: int2
    yww: int3
    ywww: int4
    ywwx: int4
    ywwy: int4
    ywwz: int4
    ywx: int3
    ywxw: int4
    ywxx: int4
    ywxy: int4
    ywxz: int4
    ywy: int3
    ywyw: int4
    ywyx: int4
    ywyy: int4
    ywyz: int4
    ywz: int3
    ywzw: int4
    ywzx: int4
    ywzy: int4
    ywzz: int4
    yx: int2
    yxw: int3
    yxww: int4
    yxwx: int4
    yxwy: int4
    yxwz: int4
    yxx: int3
    yxxw: int4
    yxxx: int4
    yxxy: int4
    yxxz: int4
    yxy: int3
    yxyw: int4
    yxyx: int4
    yxyy: int4
    yxyz: int4
    yxz: int3
    yxzw: int4
    yxzx: int4
    yxzy: int4
    yxzz: int4
    yy: int2
    yyw: int3
    yyww: int4
    yywx: int4
    yywy: int4
    yywz: int4
    yyx: int3
    yyxw: int4
    yyxx: int4
    yyxy: int4
    yyxz: int4
    yyy: int3
    yyyw: int4
    yyyx: int4
    yyyy: int4
    yyyz: int4
    yyz: int3
    yyzw: int4
    yyzx: int4
    yyzy: int4
    yyzz: int4
    yz: int2
    yzw: int3
    yzww: int4
    yzwx: int4
    yzwy: int4
    yzwz: int4
    yzx: int3
    yzxw: int4
    yzxx: int4
    yzxy: int4
    yzxz: int4
    yzy: int3
    yzyw: int4
    yzyx: int4
    yzyy: int4
    yzyz: int4
    yzz: int3
    yzzw: int4
    yzzx: int4
    yzzy: int4
    yzzz: int4
    z: int
    zw: int2
    zww: int3
    zwww: int4
    zwwx: int4
    zwwy: int4
    zwwz: int4
    zwx: int3
    zwxw: int4
    zwxx: int4
    zwxy: int4
    zwxz: int4
    zwy: int3
    zwyw: int4
    zwyx: int4
    zwyy: int4
    zwyz: int4
    zwz: int3
    zwzw: int4
    zwzx: int4
    zwzy: int4
    zwzz: int4
    zx: int2
    zxw: int3
    zxww: int4
    zxwx: int4
    zxwy: int4
    zxwz: int4
    zxx: int3
    zxxw: int4
    zxxx: int4
    zxxy: int4
    zxxz: int4
    zxy: int3
    zxyw: int4
    zxyx: int4
    zxyy: int4
    zxyz: int4
    zxz: int3
    zxzw: int4
    zxzx: int4
    zxzy: int4
    zxzz: int4
    zy: int2
    zyw: int3
    zyww: int4
    zywx: int4
    zywy: int4
    zywz: int4
    zyx: int3
    zyxw: int4
    zyxx: int4
    zyxy: int4
    zyxz: int4
    zyy: int3
    zyyw: int4
    zyyx: int4
    zyyy: int4
    zyyz: int4
    zyz: int3
    zyzw: int4
    zyzx: int4
    zyzy: int4
    zyzz: int4
    zz: int2
    zzw: int3
    zzww: int4
    zzwx: int4
    zzwy: int4
    zzwz: int4
    zzx: int3
    zzxw: int4
    zzxx: int4
    zzxy: int4
    zzxz: int4
    zzy: int3
    zzyw: int4
    zzyx: int4
    zzyy: int4
    zzyz: int4
    zzz: int3
    zzzw: int4
    zzzx: int4
    zzzy: int4
    zzzz: int4
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    @typing.overload
    def __add__(self, arg0: int4) -> int4:
        ...
    @typing.overload
    def __add__(self, arg0: int) -> int4:
        ...
    def __and__(self, arg0: int4) -> int4:
        ...
    def __eq__(self, arg0: int4) -> ...:
        ...
    def __ge__(self, arg0: int4) -> ...:
        ...
    def __getitem__(self, arg0: int) -> int:
        ...
    def __gt__(self, arg0: int4) -> ...:
        ...
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: int) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: int, arg1: int, arg2: int, arg3: int) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[int], pybind11_stubgen.typing_ext.FixedSize(4)]) -> None:
        ...
    def __le__(self, arg0: int4) -> ...:
        ...
    def __lt__(self, arg0: int4) -> ...:
        ...
    @typing.overload
    def __mod__(self, arg0: int4) -> int4:
        ...
    @typing.overload
    def __mod__(self, arg0: int) -> int4:
        ...
    @typing.overload
    def __mul__(self, arg0: int4) -> int4:
        ...
    @typing.overload
    def __mul__(self, arg0: int) -> int4:
        ...
    def __ne__(self, arg0: int4) -> ...:
        ...
    def __neg__(self) -> int4:
        ...
    def __or__(self, arg0: int4) -> int4:
        ...
    def __radd__(self, arg0: int) -> int4:
        ...
    def __repr__(self) -> str:
        ...
    def __rmod__(self, arg0: int) -> int4:
        ...
    def __rmul__(self, arg0: int) -> int4:
        ...
    def __rsub__(self, arg0: int) -> int4:
        ...
    def __rtruediv__(self, arg0: int) -> int4:
        ...
    def __setitem__(self, arg0: int, arg1: int) -> None:
        ...
    def __shl__(self, arg0: int) -> int4:
        ...
    def __shr__(self, arg0: int) -> int4:
        ...
    @typing.overload
    def __sub__(self, arg0: int4) -> int4:
        ...
    @typing.overload
    def __sub__(self, arg0: int) -> int4:
        ...
    @typing.overload
    def __truediv__(self, arg0: int4) -> int4:
        ...
    @typing.overload
    def __truediv__(self, arg0: int) -> int4:
        ...
    def __xor__(self, arg0: int4) -> int4:
        ...
    def clone(self) -> int4:
        ...
    @property
    def desc_(self) -> str:
        ...
class uint2(_VectorStorageuint2):
    __hash__: typing.ClassVar[None] = None
    x: int
    xx: uint2
    xy: uint2
    y: int
    yx: uint2
    yy: uint2
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    @typing.overload
    def __add__(self, arg0: uint2) -> uint2:
        ...
    @typing.overload
    def __add__(self, arg0: int) -> uint2:
        ...
    def __and__(self, arg0: uint2) -> uint2:
        ...
    def __eq__(self, arg0: uint2) -> ...:
        ...
    def __ge__(self, arg0: uint2) -> ...:
        ...
    def __getitem__(self, arg0: int) -> int:
        ...
    def __gt__(self, arg0: uint2) -> ...:
        ...
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: int) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: int, arg1: int) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[int], pybind11_stubgen.typing_ext.FixedSize(2)]) -> None:
        ...
    def __le__(self, arg0: uint2) -> ...:
        ...
    def __lt__(self, arg0: uint2) -> ...:
        ...
    @typing.overload
    def __mod__(self, arg0: uint2) -> uint2:
        ...
    @typing.overload
    def __mod__(self, arg0: int) -> uint2:
        ...
    @typing.overload
    def __mul__(self, arg0: uint2) -> uint2:
        ...
    @typing.overload
    def __mul__(self, arg0: int) -> uint2:
        ...
    def __ne__(self, arg0: uint2) -> ...:
        ...
    def __neg__(self) -> uint2:
        ...
    def __or__(self, arg0: uint2) -> uint2:
        ...
    def __radd__(self, arg0: int) -> uint2:
        ...
    def __repr__(self) -> str:
        ...
    def __rmod__(self, arg0: int) -> uint2:
        ...
    def __rmul__(self, arg0: int) -> uint2:
        ...
    def __rsub__(self, arg0: int) -> uint2:
        ...
    def __rtruediv__(self, arg0: int) -> uint2:
        ...
    def __setitem__(self, arg0: int, arg1: int) -> None:
        ...
    def __shl__(self, arg0: int) -> uint2:
        ...
    def __shr__(self, arg0: int) -> uint2:
        ...
    @typing.overload
    def __sub__(self, arg0: uint2) -> uint2:
        ...
    @typing.overload
    def __sub__(self, arg0: int) -> uint2:
        ...
    @typing.overload
    def __truediv__(self, arg0: uint2) -> uint2:
        ...
    @typing.overload
    def __truediv__(self, arg0: int) -> uint2:
        ...
    def __xor__(self, arg0: uint2) -> uint2:
        ...
    def clone(self) -> uint2:
        ...
    @property
    def desc_(self) -> str:
        ...
    @property
    def xxx(self) -> ...:
        ...
    @xxx.setter
    def xxx(*args, **kwargs):
        """
        (arg0: ocapi.uint2, arg1: ocarina::Vector<unsigned int,3>) -> None
        """
    @property
    def xxxx(self) -> ...:
        ...
    @xxxx.setter
    def xxxx(*args, **kwargs):
        """
        (arg0: ocapi.uint2, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def xxxy(self) -> ...:
        ...
    @xxxy.setter
    def xxxy(*args, **kwargs):
        """
        (arg0: ocapi.uint2, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def xxy(self) -> ...:
        ...
    @xxy.setter
    def xxy(*args, **kwargs):
        """
        (arg0: ocapi.uint2, arg1: ocarina::Vector<unsigned int,3>) -> None
        """
    @property
    def xxyx(self) -> ...:
        ...
    @xxyx.setter
    def xxyx(*args, **kwargs):
        """
        (arg0: ocapi.uint2, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def xxyy(self) -> ...:
        ...
    @xxyy.setter
    def xxyy(*args, **kwargs):
        """
        (arg0: ocapi.uint2, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def xyx(self) -> ...:
        ...
    @xyx.setter
    def xyx(*args, **kwargs):
        """
        (arg0: ocapi.uint2, arg1: ocarina::Vector<unsigned int,3>) -> None
        """
    @property
    def xyxx(self) -> ...:
        ...
    @xyxx.setter
    def xyxx(*args, **kwargs):
        """
        (arg0: ocapi.uint2, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def xyxy(self) -> ...:
        ...
    @xyxy.setter
    def xyxy(*args, **kwargs):
        """
        (arg0: ocapi.uint2, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def xyy(self) -> ...:
        ...
    @xyy.setter
    def xyy(*args, **kwargs):
        """
        (arg0: ocapi.uint2, arg1: ocarina::Vector<unsigned int,3>) -> None
        """
    @property
    def xyyx(self) -> ...:
        ...
    @xyyx.setter
    def xyyx(*args, **kwargs):
        """
        (arg0: ocapi.uint2, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def xyyy(self) -> ...:
        ...
    @xyyy.setter
    def xyyy(*args, **kwargs):
        """
        (arg0: ocapi.uint2, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yxx(self) -> ...:
        ...
    @yxx.setter
    def yxx(*args, **kwargs):
        """
        (arg0: ocapi.uint2, arg1: ocarina::Vector<unsigned int,3>) -> None
        """
    @property
    def yxxx(self) -> ...:
        ...
    @yxxx.setter
    def yxxx(*args, **kwargs):
        """
        (arg0: ocapi.uint2, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yxxy(self) -> ...:
        ...
    @yxxy.setter
    def yxxy(*args, **kwargs):
        """
        (arg0: ocapi.uint2, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yxy(self) -> ...:
        ...
    @yxy.setter
    def yxy(*args, **kwargs):
        """
        (arg0: ocapi.uint2, arg1: ocarina::Vector<unsigned int,3>) -> None
        """
    @property
    def yxyx(self) -> ...:
        ...
    @yxyx.setter
    def yxyx(*args, **kwargs):
        """
        (arg0: ocapi.uint2, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yxyy(self) -> ...:
        ...
    @yxyy.setter
    def yxyy(*args, **kwargs):
        """
        (arg0: ocapi.uint2, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yyx(self) -> ...:
        ...
    @yyx.setter
    def yyx(*args, **kwargs):
        """
        (arg0: ocapi.uint2, arg1: ocarina::Vector<unsigned int,3>) -> None
        """
    @property
    def yyxx(self) -> ...:
        ...
    @yyxx.setter
    def yyxx(*args, **kwargs):
        """
        (arg0: ocapi.uint2, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yyxy(self) -> ...:
        ...
    @yyxy.setter
    def yyxy(*args, **kwargs):
        """
        (arg0: ocapi.uint2, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yyy(self) -> ...:
        ...
    @yyy.setter
    def yyy(*args, **kwargs):
        """
        (arg0: ocapi.uint2, arg1: ocarina::Vector<unsigned int,3>) -> None
        """
    @property
    def yyyx(self) -> ...:
        ...
    @yyyx.setter
    def yyyx(*args, **kwargs):
        """
        (arg0: ocapi.uint2, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yyyy(self) -> ...:
        ...
    @yyyy.setter
    def yyyy(*args, **kwargs):
        """
        (arg0: ocapi.uint2, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
class uint3(_VectorStorageuint3):
    __hash__: typing.ClassVar[None] = None
    x: int
    xx: uint2
    xxx: uint3
    xxy: uint3
    xxz: uint3
    xy: uint2
    xyx: uint3
    xyy: uint3
    xyz: uint3
    xz: uint2
    xzx: uint3
    xzy: uint3
    xzz: uint3
    y: int
    yx: uint2
    yxx: uint3
    yxy: uint3
    yxz: uint3
    yy: uint2
    yyx: uint3
    yyy: uint3
    yyz: uint3
    yz: uint2
    yzx: uint3
    yzy: uint3
    yzz: uint3
    z: int
    zx: uint2
    zxx: uint3
    zxy: uint3
    zxz: uint3
    zy: uint2
    zyx: uint3
    zyy: uint3
    zyz: uint3
    zz: uint2
    zzx: uint3
    zzy: uint3
    zzz: uint3
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    @typing.overload
    def __add__(self, arg0: uint3) -> uint3:
        ...
    @typing.overload
    def __add__(self, arg0: int) -> uint3:
        ...
    def __and__(self, arg0: uint3) -> uint3:
        ...
    def __eq__(self, arg0: uint3) -> ...:
        ...
    def __ge__(self, arg0: uint3) -> ...:
        ...
    def __getitem__(self, arg0: int) -> int:
        ...
    def __gt__(self, arg0: uint3) -> ...:
        ...
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: int) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: int, arg1: int, arg2: int) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[int], pybind11_stubgen.typing_ext.FixedSize(3)]) -> None:
        ...
    def __le__(self, arg0: uint3) -> ...:
        ...
    def __lt__(self, arg0: uint3) -> ...:
        ...
    @typing.overload
    def __mod__(self, arg0: uint3) -> uint3:
        ...
    @typing.overload
    def __mod__(self, arg0: int) -> uint3:
        ...
    @typing.overload
    def __mul__(self, arg0: uint3) -> uint3:
        ...
    @typing.overload
    def __mul__(self, arg0: int) -> uint3:
        ...
    def __ne__(self, arg0: uint3) -> ...:
        ...
    def __neg__(self) -> uint3:
        ...
    def __or__(self, arg0: uint3) -> uint3:
        ...
    def __radd__(self, arg0: int) -> uint3:
        ...
    def __repr__(self) -> str:
        ...
    def __rmod__(self, arg0: int) -> uint3:
        ...
    def __rmul__(self, arg0: int) -> uint3:
        ...
    def __rsub__(self, arg0: int) -> uint3:
        ...
    def __rtruediv__(self, arg0: int) -> uint3:
        ...
    def __setitem__(self, arg0: int, arg1: int) -> None:
        ...
    def __shl__(self, arg0: int) -> uint3:
        ...
    def __shr__(self, arg0: int) -> uint3:
        ...
    @typing.overload
    def __sub__(self, arg0: uint3) -> uint3:
        ...
    @typing.overload
    def __sub__(self, arg0: int) -> uint3:
        ...
    @typing.overload
    def __truediv__(self, arg0: uint3) -> uint3:
        ...
    @typing.overload
    def __truediv__(self, arg0: int) -> uint3:
        ...
    def __xor__(self, arg0: uint3) -> uint3:
        ...
    def clone(self) -> uint3:
        ...
    @property
    def desc_(self) -> str:
        ...
    @property
    def xxxx(self) -> ...:
        ...
    @xxxx.setter
    def xxxx(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def xxxy(self) -> ...:
        ...
    @xxxy.setter
    def xxxy(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def xxxz(self) -> ...:
        ...
    @xxxz.setter
    def xxxz(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def xxyx(self) -> ...:
        ...
    @xxyx.setter
    def xxyx(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def xxyy(self) -> ...:
        ...
    @xxyy.setter
    def xxyy(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def xxyz(self) -> ...:
        ...
    @xxyz.setter
    def xxyz(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def xxzx(self) -> ...:
        ...
    @xxzx.setter
    def xxzx(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def xxzy(self) -> ...:
        ...
    @xxzy.setter
    def xxzy(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def xxzz(self) -> ...:
        ...
    @xxzz.setter
    def xxzz(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def xyxx(self) -> ...:
        ...
    @xyxx.setter
    def xyxx(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def xyxy(self) -> ...:
        ...
    @xyxy.setter
    def xyxy(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def xyxz(self) -> ...:
        ...
    @xyxz.setter
    def xyxz(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def xyyx(self) -> ...:
        ...
    @xyyx.setter
    def xyyx(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def xyyy(self) -> ...:
        ...
    @xyyy.setter
    def xyyy(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def xyyz(self) -> ...:
        ...
    @xyyz.setter
    def xyyz(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def xyzx(self) -> ...:
        ...
    @xyzx.setter
    def xyzx(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def xyzy(self) -> ...:
        ...
    @xyzy.setter
    def xyzy(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def xyzz(self) -> ...:
        ...
    @xyzz.setter
    def xyzz(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def xzxx(self) -> ...:
        ...
    @xzxx.setter
    def xzxx(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def xzxy(self) -> ...:
        ...
    @xzxy.setter
    def xzxy(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def xzxz(self) -> ...:
        ...
    @xzxz.setter
    def xzxz(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def xzyx(self) -> ...:
        ...
    @xzyx.setter
    def xzyx(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def xzyy(self) -> ...:
        ...
    @xzyy.setter
    def xzyy(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def xzyz(self) -> ...:
        ...
    @xzyz.setter
    def xzyz(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def xzzx(self) -> ...:
        ...
    @xzzx.setter
    def xzzx(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def xzzy(self) -> ...:
        ...
    @xzzy.setter
    def xzzy(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def xzzz(self) -> ...:
        ...
    @xzzz.setter
    def xzzz(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yxxx(self) -> ...:
        ...
    @yxxx.setter
    def yxxx(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yxxy(self) -> ...:
        ...
    @yxxy.setter
    def yxxy(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yxxz(self) -> ...:
        ...
    @yxxz.setter
    def yxxz(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yxyx(self) -> ...:
        ...
    @yxyx.setter
    def yxyx(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yxyy(self) -> ...:
        ...
    @yxyy.setter
    def yxyy(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yxyz(self) -> ...:
        ...
    @yxyz.setter
    def yxyz(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yxzx(self) -> ...:
        ...
    @yxzx.setter
    def yxzx(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yxzy(self) -> ...:
        ...
    @yxzy.setter
    def yxzy(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yxzz(self) -> ...:
        ...
    @yxzz.setter
    def yxzz(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yyxx(self) -> ...:
        ...
    @yyxx.setter
    def yyxx(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yyxy(self) -> ...:
        ...
    @yyxy.setter
    def yyxy(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yyxz(self) -> ...:
        ...
    @yyxz.setter
    def yyxz(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yyyx(self) -> ...:
        ...
    @yyyx.setter
    def yyyx(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yyyy(self) -> ...:
        ...
    @yyyy.setter
    def yyyy(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yyyz(self) -> ...:
        ...
    @yyyz.setter
    def yyyz(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yyzx(self) -> ...:
        ...
    @yyzx.setter
    def yyzx(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yyzy(self) -> ...:
        ...
    @yyzy.setter
    def yyzy(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yyzz(self) -> ...:
        ...
    @yyzz.setter
    def yyzz(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yzxx(self) -> ...:
        ...
    @yzxx.setter
    def yzxx(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yzxy(self) -> ...:
        ...
    @yzxy.setter
    def yzxy(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yzxz(self) -> ...:
        ...
    @yzxz.setter
    def yzxz(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yzyx(self) -> ...:
        ...
    @yzyx.setter
    def yzyx(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yzyy(self) -> ...:
        ...
    @yzyy.setter
    def yzyy(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yzyz(self) -> ...:
        ...
    @yzyz.setter
    def yzyz(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yzzx(self) -> ...:
        ...
    @yzzx.setter
    def yzzx(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yzzy(self) -> ...:
        ...
    @yzzy.setter
    def yzzy(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def yzzz(self) -> ...:
        ...
    @yzzz.setter
    def yzzz(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def zxxx(self) -> ...:
        ...
    @zxxx.setter
    def zxxx(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def zxxy(self) -> ...:
        ...
    @zxxy.setter
    def zxxy(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def zxxz(self) -> ...:
        ...
    @zxxz.setter
    def zxxz(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def zxyx(self) -> ...:
        ...
    @zxyx.setter
    def zxyx(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def zxyy(self) -> ...:
        ...
    @zxyy.setter
    def zxyy(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def zxyz(self) -> ...:
        ...
    @zxyz.setter
    def zxyz(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def zxzx(self) -> ...:
        ...
    @zxzx.setter
    def zxzx(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def zxzy(self) -> ...:
        ...
    @zxzy.setter
    def zxzy(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def zxzz(self) -> ...:
        ...
    @zxzz.setter
    def zxzz(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def zyxx(self) -> ...:
        ...
    @zyxx.setter
    def zyxx(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def zyxy(self) -> ...:
        ...
    @zyxy.setter
    def zyxy(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def zyxz(self) -> ...:
        ...
    @zyxz.setter
    def zyxz(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def zyyx(self) -> ...:
        ...
    @zyyx.setter
    def zyyx(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def zyyy(self) -> ...:
        ...
    @zyyy.setter
    def zyyy(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def zyyz(self) -> ...:
        ...
    @zyyz.setter
    def zyyz(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def zyzx(self) -> ...:
        ...
    @zyzx.setter
    def zyzx(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def zyzy(self) -> ...:
        ...
    @zyzy.setter
    def zyzy(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def zyzz(self) -> ...:
        ...
    @zyzz.setter
    def zyzz(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def zzxx(self) -> ...:
        ...
    @zzxx.setter
    def zzxx(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def zzxy(self) -> ...:
        ...
    @zzxy.setter
    def zzxy(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def zzxz(self) -> ...:
        ...
    @zzxz.setter
    def zzxz(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def zzyx(self) -> ...:
        ...
    @zzyx.setter
    def zzyx(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def zzyy(self) -> ...:
        ...
    @zzyy.setter
    def zzyy(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def zzyz(self) -> ...:
        ...
    @zzyz.setter
    def zzyz(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def zzzx(self) -> ...:
        ...
    @zzzx.setter
    def zzzx(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def zzzy(self) -> ...:
        ...
    @zzzy.setter
    def zzzy(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
    @property
    def zzzz(self) -> ...:
        ...
    @zzzz.setter
    def zzzz(*args, **kwargs):
        """
        (arg0: ocapi.uint3, arg1: ocarina::Vector<unsigned int,4>) -> None
        """
class uint4(_VectorStorageuint4):
    __hash__: typing.ClassVar[None] = None
    w: int
    ww: uint2
    www: uint3
    wwww: uint4
    wwwx: uint4
    wwwy: uint4
    wwwz: uint4
    wwx: uint3
    wwxw: uint4
    wwxx: uint4
    wwxy: uint4
    wwxz: uint4
    wwy: uint3
    wwyw: uint4
    wwyx: uint4
    wwyy: uint4
    wwyz: uint4
    wwz: uint3
    wwzw: uint4
    wwzx: uint4
    wwzy: uint4
    wwzz: uint4
    wx: uint2
    wxw: uint3
    wxww: uint4
    wxwx: uint4
    wxwy: uint4
    wxwz: uint4
    wxx: uint3
    wxxw: uint4
    wxxx: uint4
    wxxy: uint4
    wxxz: uint4
    wxy: uint3
    wxyw: uint4
    wxyx: uint4
    wxyy: uint4
    wxyz: uint4
    wxz: uint3
    wxzw: uint4
    wxzx: uint4
    wxzy: uint4
    wxzz: uint4
    wy: uint2
    wyw: uint3
    wyww: uint4
    wywx: uint4
    wywy: uint4
    wywz: uint4
    wyx: uint3
    wyxw: uint4
    wyxx: uint4
    wyxy: uint4
    wyxz: uint4
    wyy: uint3
    wyyw: uint4
    wyyx: uint4
    wyyy: uint4
    wyyz: uint4
    wyz: uint3
    wyzw: uint4
    wyzx: uint4
    wyzy: uint4
    wyzz: uint4
    wz: uint2
    wzw: uint3
    wzww: uint4
    wzwx: uint4
    wzwy: uint4
    wzwz: uint4
    wzx: uint3
    wzxw: uint4
    wzxx: uint4
    wzxy: uint4
    wzxz: uint4
    wzy: uint3
    wzyw: uint4
    wzyx: uint4
    wzyy: uint4
    wzyz: uint4
    wzz: uint3
    wzzw: uint4
    wzzx: uint4
    wzzy: uint4
    wzzz: uint4
    x: int
    xw: uint2
    xww: uint3
    xwww: uint4
    xwwx: uint4
    xwwy: uint4
    xwwz: uint4
    xwx: uint3
    xwxw: uint4
    xwxx: uint4
    xwxy: uint4
    xwxz: uint4
    xwy: uint3
    xwyw: uint4
    xwyx: uint4
    xwyy: uint4
    xwyz: uint4
    xwz: uint3
    xwzw: uint4
    xwzx: uint4
    xwzy: uint4
    xwzz: uint4
    xx: uint2
    xxw: uint3
    xxww: uint4
    xxwx: uint4
    xxwy: uint4
    xxwz: uint4
    xxx: uint3
    xxxw: uint4
    xxxx: uint4
    xxxy: uint4
    xxxz: uint4
    xxy: uint3
    xxyw: uint4
    xxyx: uint4
    xxyy: uint4
    xxyz: uint4
    xxz: uint3
    xxzw: uint4
    xxzx: uint4
    xxzy: uint4
    xxzz: uint4
    xy: uint2
    xyw: uint3
    xyww: uint4
    xywx: uint4
    xywy: uint4
    xywz: uint4
    xyx: uint3
    xyxw: uint4
    xyxx: uint4
    xyxy: uint4
    xyxz: uint4
    xyy: uint3
    xyyw: uint4
    xyyx: uint4
    xyyy: uint4
    xyyz: uint4
    xyz: uint3
    xyzw: uint4
    xyzx: uint4
    xyzy: uint4
    xyzz: uint4
    xz: uint2
    xzw: uint3
    xzww: uint4
    xzwx: uint4
    xzwy: uint4
    xzwz: uint4
    xzx: uint3
    xzxw: uint4
    xzxx: uint4
    xzxy: uint4
    xzxz: uint4
    xzy: uint3
    xzyw: uint4
    xzyx: uint4
    xzyy: uint4
    xzyz: uint4
    xzz: uint3
    xzzw: uint4
    xzzx: uint4
    xzzy: uint4
    xzzz: uint4
    y: int
    yw: uint2
    yww: uint3
    ywww: uint4
    ywwx: uint4
    ywwy: uint4
    ywwz: uint4
    ywx: uint3
    ywxw: uint4
    ywxx: uint4
    ywxy: uint4
    ywxz: uint4
    ywy: uint3
    ywyw: uint4
    ywyx: uint4
    ywyy: uint4
    ywyz: uint4
    ywz: uint3
    ywzw: uint4
    ywzx: uint4
    ywzy: uint4
    ywzz: uint4
    yx: uint2
    yxw: uint3
    yxww: uint4
    yxwx: uint4
    yxwy: uint4
    yxwz: uint4
    yxx: uint3
    yxxw: uint4
    yxxx: uint4
    yxxy: uint4
    yxxz: uint4
    yxy: uint3
    yxyw: uint4
    yxyx: uint4
    yxyy: uint4
    yxyz: uint4
    yxz: uint3
    yxzw: uint4
    yxzx: uint4
    yxzy: uint4
    yxzz: uint4
    yy: uint2
    yyw: uint3
    yyww: uint4
    yywx: uint4
    yywy: uint4
    yywz: uint4
    yyx: uint3
    yyxw: uint4
    yyxx: uint4
    yyxy: uint4
    yyxz: uint4
    yyy: uint3
    yyyw: uint4
    yyyx: uint4
    yyyy: uint4
    yyyz: uint4
    yyz: uint3
    yyzw: uint4
    yyzx: uint4
    yyzy: uint4
    yyzz: uint4
    yz: uint2
    yzw: uint3
    yzww: uint4
    yzwx: uint4
    yzwy: uint4
    yzwz: uint4
    yzx: uint3
    yzxw: uint4
    yzxx: uint4
    yzxy: uint4
    yzxz: uint4
    yzy: uint3
    yzyw: uint4
    yzyx: uint4
    yzyy: uint4
    yzyz: uint4
    yzz: uint3
    yzzw: uint4
    yzzx: uint4
    yzzy: uint4
    yzzz: uint4
    z: int
    zw: uint2
    zww: uint3
    zwww: uint4
    zwwx: uint4
    zwwy: uint4
    zwwz: uint4
    zwx: uint3
    zwxw: uint4
    zwxx: uint4
    zwxy: uint4
    zwxz: uint4
    zwy: uint3
    zwyw: uint4
    zwyx: uint4
    zwyy: uint4
    zwyz: uint4
    zwz: uint3
    zwzw: uint4
    zwzx: uint4
    zwzy: uint4
    zwzz: uint4
    zx: uint2
    zxw: uint3
    zxww: uint4
    zxwx: uint4
    zxwy: uint4
    zxwz: uint4
    zxx: uint3
    zxxw: uint4
    zxxx: uint4
    zxxy: uint4
    zxxz: uint4
    zxy: uint3
    zxyw: uint4
    zxyx: uint4
    zxyy: uint4
    zxyz: uint4
    zxz: uint3
    zxzw: uint4
    zxzx: uint4
    zxzy: uint4
    zxzz: uint4
    zy: uint2
    zyw: uint3
    zyww: uint4
    zywx: uint4
    zywy: uint4
    zywz: uint4
    zyx: uint3
    zyxw: uint4
    zyxx: uint4
    zyxy: uint4
    zyxz: uint4
    zyy: uint3
    zyyw: uint4
    zyyx: uint4
    zyyy: uint4
    zyyz: uint4
    zyz: uint3
    zyzw: uint4
    zyzx: uint4
    zyzy: uint4
    zyzz: uint4
    zz: uint2
    zzw: uint3
    zzww: uint4
    zzwx: uint4
    zzwy: uint4
    zzwz: uint4
    zzx: uint3
    zzxw: uint4
    zzxx: uint4
    zzxy: uint4
    zzxz: uint4
    zzy: uint3
    zzyw: uint4
    zzyx: uint4
    zzyy: uint4
    zzyz: uint4
    zzz: uint3
    zzzw: uint4
    zzzx: uint4
    zzzy: uint4
    zzzz: uint4
    @staticmethod
    def _pybind11_conduit_v1_(*args, **kwargs):
        ...
    @typing.overload
    def __add__(self, arg0: uint4) -> uint4:
        ...
    @typing.overload
    def __add__(self, arg0: int) -> uint4:
        ...
    def __and__(self, arg0: uint4) -> uint4:
        ...
    def __eq__(self, arg0: uint4) -> ...:
        ...
    def __ge__(self, arg0: uint4) -> ...:
        ...
    def __getitem__(self, arg0: int) -> int:
        ...
    def __gt__(self, arg0: uint4) -> ...:
        ...
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: int) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: int, arg1: int, arg2: int, arg3: int) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.Annotated[list[int], pybind11_stubgen.typing_ext.FixedSize(4)]) -> None:
        ...
    def __le__(self, arg0: uint4) -> ...:
        ...
    def __lt__(self, arg0: uint4) -> ...:
        ...
    @typing.overload
    def __mod__(self, arg0: uint4) -> uint4:
        ...
    @typing.overload
    def __mod__(self, arg0: int) -> uint4:
        ...
    @typing.overload
    def __mul__(self, arg0: uint4) -> uint4:
        ...
    @typing.overload
    def __mul__(self, arg0: int) -> uint4:
        ...
    def __ne__(self, arg0: uint4) -> ...:
        ...
    def __neg__(self) -> uint4:
        ...
    def __or__(self, arg0: uint4) -> uint4:
        ...
    def __radd__(self, arg0: int) -> uint4:
        ...
    def __repr__(self) -> str:
        ...
    def __rmod__(self, arg0: int) -> uint4:
        ...
    def __rmul__(self, arg0: int) -> uint4:
        ...
    def __rsub__(self, arg0: int) -> uint4:
        ...
    def __rtruediv__(self, arg0: int) -> uint4:
        ...
    def __setitem__(self, arg0: int, arg1: int) -> None:
        ...
    def __shl__(self, arg0: int) -> uint4:
        ...
    def __shr__(self, arg0: int) -> uint4:
        ...
    @typing.overload
    def __sub__(self, arg0: uint4) -> uint4:
        ...
    @typing.overload
    def __sub__(self, arg0: int) -> uint4:
        ...
    @typing.overload
    def __truediv__(self, arg0: uint4) -> uint4:
        ...
    @typing.overload
    def __truediv__(self, arg0: int) -> uint4:
        ...
    def __xor__(self, arg0: uint4) -> uint4:
        ...
    def clone(self) -> uint4:
        ...
    @property
    def desc_(self) -> str:
        ...
@typing.overload
def abs(arg0: float2) -> float2:
    ...
@typing.overload
def abs(arg0: int2) -> int2:
    ...
@typing.overload
def abs(arg0: float3) -> float3:
    ...
@typing.overload
def abs(arg0: int3) -> int3:
    ...
@typing.overload
def abs(arg0: float4) -> float4:
    ...
@typing.overload
def abs(arg0: int4) -> int4:
    ...
@typing.overload
def abs(arg0: float2x2) -> float2x2:
    ...
@typing.overload
def abs(arg0: float2x3) -> float2x3:
    ...
@typing.overload
def abs(arg0: float2x4) -> float2x4:
    ...
@typing.overload
def abs(arg0: float3x2) -> float3x2:
    ...
@typing.overload
def abs(arg0: float3x3) -> float3x3:
    ...
@typing.overload
def abs(arg0: float3x4) -> float3x4:
    ...
@typing.overload
def abs(arg0: float4x2) -> float4x2:
    ...
@typing.overload
def abs(arg0: float4x3) -> float4x3:
    ...
@typing.overload
def abs(arg0: float4x4) -> float4x4:
    ...
@typing.overload
def acos(arg0: float2) -> float2:
    ...
@typing.overload
def acos(arg0: float3) -> float3:
    ...
@typing.overload
def acos(arg0: float4) -> float4:
    ...
@typing.overload
def acos(arg0: float2x2) -> float2x2:
    ...
@typing.overload
def acos(arg0: float2x3) -> float2x3:
    ...
@typing.overload
def acos(arg0: float2x4) -> float2x4:
    ...
@typing.overload
def acos(arg0: float3x2) -> float3x2:
    ...
@typing.overload
def acos(arg0: float3x3) -> float3x3:
    ...
@typing.overload
def acos(arg0: float3x4) -> float3x4:
    ...
@typing.overload
def acos(arg0: float4x2) -> float4x2:
    ...
@typing.overload
def acos(arg0: float4x3) -> float4x3:
    ...
@typing.overload
def acos(arg0: float4x4) -> float4x4:
    ...
@typing.overload
def acosh(arg0: float2) -> float2:
    ...
@typing.overload
def acosh(arg0: float3) -> float3:
    ...
@typing.overload
def acosh(arg0: float4) -> float4:
    ...
@typing.overload
def acosh(arg0: float2x2) -> float2x2:
    ...
@typing.overload
def acosh(arg0: float2x3) -> float2x3:
    ...
@typing.overload
def acosh(arg0: float2x4) -> float2x4:
    ...
@typing.overload
def acosh(arg0: float3x2) -> float3x2:
    ...
@typing.overload
def acosh(arg0: float3x3) -> float3x3:
    ...
@typing.overload
def acosh(arg0: float3x4) -> float3x4:
    ...
@typing.overload
def acosh(arg0: float4x2) -> float4x2:
    ...
@typing.overload
def acosh(arg0: float4x3) -> float4x3:
    ...
@typing.overload
def acosh(arg0: float4x4) -> float4x4:
    ...
@typing.overload
def all(arg0: bool2) -> bool:
    ...
@typing.overload
def all(arg0: bool3) -> bool:
    ...
@typing.overload
def all(arg0: bool4) -> bool:
    ...
@typing.overload
def any(arg0: bool2) -> bool:
    ...
@typing.overload
def any(arg0: bool3) -> bool:
    ...
@typing.overload
def any(arg0: bool4) -> bool:
    ...
@typing.overload
def as_float(arg0: int) -> float:
    ...
@typing.overload
def as_float(arg0: int) -> float:
    ...
@typing.overload
def as_float2(*args, **kwargs) -> float2:
    ...
@typing.overload
def as_float2(*args, **kwargs) -> float2:
    ...
@typing.overload
def as_float3(*args, **kwargs) -> float3:
    ...
@typing.overload
def as_float3(*args, **kwargs) -> float3:
    ...
@typing.overload
def as_float4(*args, **kwargs) -> float4:
    ...
@typing.overload
def as_float4(*args, **kwargs) -> float4:
    ...
@typing.overload
def as_int(arg0: int) -> int:
    ...
@typing.overload
def as_int(arg0: float) -> int:
    ...
@typing.overload
def as_int2(arg0: uint2) -> int2:
    ...
@typing.overload
def as_int2(arg0: float2) -> int2:
    ...
@typing.overload
def as_int3(arg0: uint3) -> int3:
    ...
@typing.overload
def as_int3(arg0: float3) -> int3:
    ...
@typing.overload
def as_int4(arg0: uint4) -> int4:
    ...
@typing.overload
def as_int4(arg0: float4) -> int4:
    ...
@typing.overload
def as_uint(arg0: int) -> int:
    ...
@typing.overload
def as_uint(arg0: float) -> int:
    ...
@typing.overload
def as_uint2(*args, **kwargs) -> uint2:
    ...
@typing.overload
def as_uint2(arg0: float2) -> uint2:
    ...
@typing.overload
def as_uint3(*args, **kwargs) -> uint3:
    ...
@typing.overload
def as_uint3(arg0: float3) -> uint3:
    ...
@typing.overload
def as_uint4(*args, **kwargs) -> uint4:
    ...
@typing.overload
def as_uint4(arg0: float4) -> uint4:
    ...
@typing.overload
def asin(arg0: float2) -> float2:
    ...
@typing.overload
def asin(arg0: float3) -> float3:
    ...
@typing.overload
def asin(arg0: float4) -> float4:
    ...
@typing.overload
def asin(arg0: float2x2) -> float2x2:
    ...
@typing.overload
def asin(arg0: float2x3) -> float2x3:
    ...
@typing.overload
def asin(arg0: float2x4) -> float2x4:
    ...
@typing.overload
def asin(arg0: float3x2) -> float3x2:
    ...
@typing.overload
def asin(arg0: float3x3) -> float3x3:
    ...
@typing.overload
def asin(arg0: float3x4) -> float3x4:
    ...
@typing.overload
def asin(arg0: float4x2) -> float4x2:
    ...
@typing.overload
def asin(arg0: float4x3) -> float4x3:
    ...
@typing.overload
def asin(arg0: float4x4) -> float4x4:
    ...
@typing.overload
def asinh(arg0: float2) -> float2:
    ...
@typing.overload
def asinh(arg0: float3) -> float3:
    ...
@typing.overload
def asinh(arg0: float4) -> float4:
    ...
@typing.overload
def asinh(arg0: float2x2) -> float2x2:
    ...
@typing.overload
def asinh(arg0: float2x3) -> float2x3:
    ...
@typing.overload
def asinh(arg0: float2x4) -> float2x4:
    ...
@typing.overload
def asinh(arg0: float3x2) -> float3x2:
    ...
@typing.overload
def asinh(arg0: float3x3) -> float3x3:
    ...
@typing.overload
def asinh(arg0: float3x4) -> float3x4:
    ...
@typing.overload
def asinh(arg0: float4x2) -> float4x2:
    ...
@typing.overload
def asinh(arg0: float4x3) -> float4x3:
    ...
@typing.overload
def asinh(arg0: float4x4) -> float4x4:
    ...
@typing.overload
def atan(arg0: float2) -> float2:
    ...
@typing.overload
def atan(arg0: float3) -> float3:
    ...
@typing.overload
def atan(arg0: float4) -> float4:
    ...
@typing.overload
def atan(arg0: float2x2) -> float2x2:
    ...
@typing.overload
def atan(arg0: float2x3) -> float2x3:
    ...
@typing.overload
def atan(arg0: float2x4) -> float2x4:
    ...
@typing.overload
def atan(arg0: float3x2) -> float3x2:
    ...
@typing.overload
def atan(arg0: float3x3) -> float3x3:
    ...
@typing.overload
def atan(arg0: float3x4) -> float3x4:
    ...
@typing.overload
def atan(arg0: float4x2) -> float4x2:
    ...
@typing.overload
def atan(arg0: float4x3) -> float4x3:
    ...
@typing.overload
def atan(arg0: float4x4) -> float4x4:
    ...
@typing.overload
def atan2(arg0: float2, arg1: float2) -> float2:
    ...
@typing.overload
def atan2(arg0: float3, arg1: float3) -> float3:
    ...
@typing.overload
def atan2(arg0: float4, arg1: float4) -> float4:
    ...
@typing.overload
def atanh(arg0: float2) -> float2:
    ...
@typing.overload
def atanh(arg0: float3) -> float3:
    ...
@typing.overload
def atanh(arg0: float4) -> float4:
    ...
@typing.overload
def atanh(arg0: float2x2) -> float2x2:
    ...
@typing.overload
def atanh(arg0: float2x3) -> float2x3:
    ...
@typing.overload
def atanh(arg0: float2x4) -> float2x4:
    ...
@typing.overload
def atanh(arg0: float3x2) -> float3x2:
    ...
@typing.overload
def atanh(arg0: float3x3) -> float3x3:
    ...
@typing.overload
def atanh(arg0: float3x4) -> float3x4:
    ...
@typing.overload
def atanh(arg0: float4x2) -> float4x2:
    ...
@typing.overload
def atanh(arg0: float4x3) -> float4x3:
    ...
@typing.overload
def atanh(arg0: float4x4) -> float4x4:
    ...
@typing.overload
def ceil(arg0: float2) -> float2:
    ...
@typing.overload
def ceil(arg0: float3) -> float3:
    ...
@typing.overload
def ceil(arg0: float4) -> float4:
    ...
@typing.overload
def ceil(arg0: float2x2) -> float2x2:
    ...
@typing.overload
def ceil(arg0: float2x3) -> float2x3:
    ...
@typing.overload
def ceil(arg0: float2x4) -> float2x4:
    ...
@typing.overload
def ceil(arg0: float3x2) -> float3x2:
    ...
@typing.overload
def ceil(arg0: float3x3) -> float3x3:
    ...
@typing.overload
def ceil(arg0: float3x4) -> float3x4:
    ...
@typing.overload
def ceil(arg0: float4x2) -> float4x2:
    ...
@typing.overload
def ceil(arg0: float4x3) -> float4x3:
    ...
@typing.overload
def ceil(arg0: float4x4) -> float4x4:
    ...
@typing.overload
def clamp(arg0: float2, arg1: float2, arg2: float2) -> float2:
    ...
@typing.overload
def clamp(arg0: uint2, arg1: uint2, arg2: uint2) -> uint2:
    ...
@typing.overload
def clamp(arg0: int2, arg1: int2, arg2: int2) -> int2:
    ...
@typing.overload
def clamp(arg0: float3, arg1: float3, arg2: float3) -> float3:
    ...
@typing.overload
def clamp(arg0: uint3, arg1: uint3, arg2: uint3) -> uint3:
    ...
@typing.overload
def clamp(arg0: int3, arg1: int3, arg2: int3) -> int3:
    ...
@typing.overload
def clamp(arg0: float4, arg1: float4, arg2: float4) -> float4:
    ...
@typing.overload
def clamp(arg0: uint4, arg1: uint4, arg2: uint4) -> uint4:
    ...
@typing.overload
def clamp(arg0: int4, arg1: int4, arg2: int4) -> int4:
    ...
@typing.overload
def cos(arg0: float2) -> float2:
    ...
@typing.overload
def cos(arg0: float3) -> float3:
    ...
@typing.overload
def cos(arg0: float4) -> float4:
    ...
@typing.overload
def cos(arg0: float2x2) -> float2x2:
    ...
@typing.overload
def cos(arg0: float2x3) -> float2x3:
    ...
@typing.overload
def cos(arg0: float2x4) -> float2x4:
    ...
@typing.overload
def cos(arg0: float3x2) -> float3x2:
    ...
@typing.overload
def cos(arg0: float3x3) -> float3x3:
    ...
@typing.overload
def cos(arg0: float3x4) -> float3x4:
    ...
@typing.overload
def cos(arg0: float4x2) -> float4x2:
    ...
@typing.overload
def cos(arg0: float4x3) -> float4x3:
    ...
@typing.overload
def cos(arg0: float4x4) -> float4x4:
    ...
@typing.overload
def cosh(arg0: float2) -> float2:
    ...
@typing.overload
def cosh(arg0: float3) -> float3:
    ...
@typing.overload
def cosh(arg0: float4) -> float4:
    ...
@typing.overload
def cosh(arg0: float2x2) -> float2x2:
    ...
@typing.overload
def cosh(arg0: float2x3) -> float2x3:
    ...
@typing.overload
def cosh(arg0: float2x4) -> float2x4:
    ...
@typing.overload
def cosh(arg0: float3x2) -> float3x2:
    ...
@typing.overload
def cosh(arg0: float3x3) -> float3x3:
    ...
@typing.overload
def cosh(arg0: float3x4) -> float3x4:
    ...
@typing.overload
def cosh(arg0: float4x2) -> float4x2:
    ...
@typing.overload
def cosh(arg0: float4x3) -> float4x3:
    ...
@typing.overload
def cosh(arg0: float4x4) -> float4x4:
    ...
@typing.overload
def cross(arg0: float3, arg1: float3) -> float3:
    ...
@typing.overload
def cross(arg0: uint3, arg1: uint3) -> uint3:
    ...
@typing.overload
def cross(arg0: int3, arg1: int3) -> int3:
    ...
@typing.overload
def degrees(arg0: float2) -> float2:
    ...
@typing.overload
def degrees(arg0: float3) -> float3:
    ...
@typing.overload
def degrees(arg0: float4) -> float4:
    ...
@typing.overload
def degrees(arg0: float2x2) -> float2x2:
    ...
@typing.overload
def degrees(arg0: float2x3) -> float2x3:
    ...
@typing.overload
def degrees(arg0: float2x4) -> float2x4:
    ...
@typing.overload
def degrees(arg0: float3x2) -> float3x2:
    ...
@typing.overload
def degrees(arg0: float3x3) -> float3x3:
    ...
@typing.overload
def degrees(arg0: float3x4) -> float3x4:
    ...
@typing.overload
def degrees(arg0: float4x2) -> float4x2:
    ...
@typing.overload
def degrees(arg0: float4x3) -> float4x3:
    ...
@typing.overload
def degrees(arg0: float4x4) -> float4x4:
    ...
@typing.overload
def determinant(arg0: float2x2) -> float:
    ...
@typing.overload
def determinant(arg0: float3x3) -> float:
    ...
@typing.overload
def determinant(arg0: float4x4) -> float:
    ...
@typing.overload
def distance(arg0: float2, arg1: float2) -> float:
    ...
@typing.overload
def distance(arg0: uint2, arg1: uint2) -> float:
    ...
@typing.overload
def distance(arg0: int2, arg1: int2) -> float:
    ...
@typing.overload
def distance(arg0: float3, arg1: float3) -> float:
    ...
@typing.overload
def distance(arg0: uint3, arg1: uint3) -> float:
    ...
@typing.overload
def distance(arg0: int3, arg1: int3) -> float:
    ...
@typing.overload
def distance(arg0: float4, arg1: float4) -> float:
    ...
@typing.overload
def distance(arg0: uint4, arg1: uint4) -> float:
    ...
@typing.overload
def distance(arg0: int4, arg1: int4) -> float:
    ...
@typing.overload
def distance_squared(arg0: float2, arg1: float2) -> float:
    ...
@typing.overload
def distance_squared(arg0: uint2, arg1: uint2) -> int:
    ...
@typing.overload
def distance_squared(arg0: int2, arg1: int2) -> int:
    ...
@typing.overload
def distance_squared(arg0: float3, arg1: float3) -> float:
    ...
@typing.overload
def distance_squared(arg0: uint3, arg1: uint3) -> int:
    ...
@typing.overload
def distance_squared(arg0: int3, arg1: int3) -> int:
    ...
@typing.overload
def distance_squared(arg0: float4, arg1: float4) -> float:
    ...
@typing.overload
def distance_squared(arg0: uint4, arg1: uint4) -> int:
    ...
@typing.overload
def distance_squared(arg0: int4, arg1: int4) -> int:
    ...
@typing.overload
def dot(arg0: float2, arg1: float2) -> float:
    ...
@typing.overload
def dot(arg0: uint2, arg1: uint2) -> int:
    ...
@typing.overload
def dot(arg0: int2, arg1: int2) -> int:
    ...
@typing.overload
def dot(arg0: float3, arg1: float3) -> float:
    ...
@typing.overload
def dot(arg0: uint3, arg1: uint3) -> int:
    ...
@typing.overload
def dot(arg0: int3, arg1: int3) -> int:
    ...
@typing.overload
def dot(arg0: float4, arg1: float4) -> float:
    ...
@typing.overload
def dot(arg0: uint4, arg1: uint4) -> int:
    ...
@typing.overload
def dot(arg0: int4, arg1: int4) -> int:
    ...
@typing.overload
def exp(arg0: float2) -> float2:
    ...
@typing.overload
def exp(arg0: float3) -> float3:
    ...
@typing.overload
def exp(arg0: float4) -> float4:
    ...
@typing.overload
def exp(arg0: float2x2) -> float2x2:
    ...
@typing.overload
def exp(arg0: float2x3) -> float2x3:
    ...
@typing.overload
def exp(arg0: float2x4) -> float2x4:
    ...
@typing.overload
def exp(arg0: float3x2) -> float3x2:
    ...
@typing.overload
def exp(arg0: float3x3) -> float3x3:
    ...
@typing.overload
def exp(arg0: float3x4) -> float3x4:
    ...
@typing.overload
def exp(arg0: float4x2) -> float4x2:
    ...
@typing.overload
def exp(arg0: float4x3) -> float4x3:
    ...
@typing.overload
def exp(arg0: float4x4) -> float4x4:
    ...
@typing.overload
def exp2(arg0: float2) -> float2:
    ...
@typing.overload
def exp2(arg0: float3) -> float3:
    ...
@typing.overload
def exp2(arg0: float4) -> float4:
    ...
@typing.overload
def exp2(arg0: float2x2) -> float2x2:
    ...
@typing.overload
def exp2(arg0: float2x3) -> float2x3:
    ...
@typing.overload
def exp2(arg0: float2x4) -> float2x4:
    ...
@typing.overload
def exp2(arg0: float3x2) -> float3x2:
    ...
@typing.overload
def exp2(arg0: float3x3) -> float3x3:
    ...
@typing.overload
def exp2(arg0: float3x4) -> float3x4:
    ...
@typing.overload
def exp2(arg0: float4x2) -> float4x2:
    ...
@typing.overload
def exp2(arg0: float4x3) -> float4x3:
    ...
@typing.overload
def exp2(arg0: float4x4) -> float4x4:
    ...
@typing.overload
def floor(arg0: float2) -> float2:
    ...
@typing.overload
def floor(arg0: float3) -> float3:
    ...
@typing.overload
def floor(arg0: float4) -> float4:
    ...
@typing.overload
def floor(arg0: float2x2) -> float2x2:
    ...
@typing.overload
def floor(arg0: float2x3) -> float2x3:
    ...
@typing.overload
def floor(arg0: float2x4) -> float2x4:
    ...
@typing.overload
def floor(arg0: float3x2) -> float3x2:
    ...
@typing.overload
def floor(arg0: float3x3) -> float3x3:
    ...
@typing.overload
def floor(arg0: float3x4) -> float3x4:
    ...
@typing.overload
def floor(arg0: float4x2) -> float4x2:
    ...
@typing.overload
def floor(arg0: float4x3) -> float4x3:
    ...
@typing.overload
def floor(arg0: float4x4) -> float4x4:
    ...
@typing.overload
def fma(arg0: float2, arg1: float2, arg2: float2) -> float2:
    ...
@typing.overload
def fma(arg0: float3, arg1: float3, arg2: float3) -> float3:
    ...
@typing.overload
def fma(arg0: float4, arg1: float4, arg2: float4) -> float4:
    ...
@typing.overload
def fract(arg0: float2) -> float2:
    ...
@typing.overload
def fract(arg0: float3) -> float3:
    ...
@typing.overload
def fract(arg0: float4) -> float4:
    ...
@typing.overload
def fract(arg0: float2x2) -> float2x2:
    ...
@typing.overload
def fract(arg0: float2x3) -> float2x3:
    ...
@typing.overload
def fract(arg0: float2x4) -> float2x4:
    ...
@typing.overload
def fract(arg0: float3x2) -> float3x2:
    ...
@typing.overload
def fract(arg0: float3x3) -> float3x3:
    ...
@typing.overload
def fract(arg0: float3x4) -> float3x4:
    ...
@typing.overload
def fract(arg0: float4x2) -> float4x2:
    ...
@typing.overload
def fract(arg0: float4x3) -> float4x3:
    ...
@typing.overload
def fract(arg0: float4x4) -> float4x4:
    ...
@typing.overload
def inverse(arg0: float2x2) -> float2x2:
    ...
@typing.overload
def inverse(arg0: float3x3) -> float3x3:
    ...
@typing.overload
def inverse(arg0: float4x4) -> float4x4:
    ...
@typing.overload
def isinf(arg0: float2) -> ...:
    ...
@typing.overload
def isinf(arg0: float3) -> ...:
    ...
@typing.overload
def isinf(arg0: float4) -> ...:
    ...
@typing.overload
def isinf(arg0: float2x2) -> float2x2:
    ...
@typing.overload
def isinf(arg0: float2x3) -> float2x3:
    ...
@typing.overload
def isinf(arg0: float2x4) -> float2x4:
    ...
@typing.overload
def isinf(arg0: float3x2) -> float3x2:
    ...
@typing.overload
def isinf(arg0: float3x3) -> float3x3:
    ...
@typing.overload
def isinf(arg0: float3x4) -> float3x4:
    ...
@typing.overload
def isinf(arg0: float4x2) -> float4x2:
    ...
@typing.overload
def isinf(arg0: float4x3) -> float4x3:
    ...
@typing.overload
def isinf(arg0: float4x4) -> float4x4:
    ...
@typing.overload
def isnan(arg0: float2) -> ...:
    ...
@typing.overload
def isnan(arg0: float3) -> ...:
    ...
@typing.overload
def isnan(arg0: float4) -> ...:
    ...
@typing.overload
def isnan(arg0: float2x2) -> float2x2:
    ...
@typing.overload
def isnan(arg0: float2x3) -> float2x3:
    ...
@typing.overload
def isnan(arg0: float2x4) -> float2x4:
    ...
@typing.overload
def isnan(arg0: float3x2) -> float3x2:
    ...
@typing.overload
def isnan(arg0: float3x3) -> float3x3:
    ...
@typing.overload
def isnan(arg0: float3x4) -> float3x4:
    ...
@typing.overload
def isnan(arg0: float4x2) -> float4x2:
    ...
@typing.overload
def isnan(arg0: float4x3) -> float4x3:
    ...
@typing.overload
def isnan(arg0: float4x4) -> float4x4:
    ...
@typing.overload
def length(arg0: float2) -> float:
    ...
@typing.overload
def length(arg0: float3) -> float:
    ...
@typing.overload
def length(arg0: float4) -> float:
    ...
@typing.overload
def length_squared(arg0: float2) -> float:
    ...
@typing.overload
def length_squared(arg0: float3) -> float:
    ...
@typing.overload
def length_squared(arg0: float4) -> float:
    ...
@typing.overload
def lerp(arg0: float2, arg1: float2, arg2: float2) -> float2:
    ...
@typing.overload
def lerp(arg0: float3, arg1: float3, arg2: float3) -> float3:
    ...
@typing.overload
def lerp(arg0: float4, arg1: float4, arg2: float4) -> float4:
    ...
def load_lib(arg0: str) -> None:
    ...
@typing.overload
def log(arg0: float2) -> float2:
    ...
@typing.overload
def log(arg0: float3) -> float3:
    ...
@typing.overload
def log(arg0: float4) -> float4:
    ...
@typing.overload
def log(arg0: float2x2) -> float2x2:
    ...
@typing.overload
def log(arg0: float2x3) -> float2x3:
    ...
@typing.overload
def log(arg0: float2x4) -> float2x4:
    ...
@typing.overload
def log(arg0: float3x2) -> float3x2:
    ...
@typing.overload
def log(arg0: float3x3) -> float3x3:
    ...
@typing.overload
def log(arg0: float3x4) -> float3x4:
    ...
@typing.overload
def log(arg0: float4x2) -> float4x2:
    ...
@typing.overload
def log(arg0: float4x3) -> float4x3:
    ...
@typing.overload
def log(arg0: float4x4) -> float4x4:
    ...
@typing.overload
def log10(arg0: float2) -> float2:
    ...
@typing.overload
def log10(arg0: float3) -> float3:
    ...
@typing.overload
def log10(arg0: float4) -> float4:
    ...
@typing.overload
def log10(arg0: float2x2) -> float2x2:
    ...
@typing.overload
def log10(arg0: float2x3) -> float2x3:
    ...
@typing.overload
def log10(arg0: float2x4) -> float2x4:
    ...
@typing.overload
def log10(arg0: float3x2) -> float3x2:
    ...
@typing.overload
def log10(arg0: float3x3) -> float3x3:
    ...
@typing.overload
def log10(arg0: float3x4) -> float3x4:
    ...
@typing.overload
def log10(arg0: float4x2) -> float4x2:
    ...
@typing.overload
def log10(arg0: float4x3) -> float4x3:
    ...
@typing.overload
def log10(arg0: float4x4) -> float4x4:
    ...
@typing.overload
def log2(arg0: float2) -> float2:
    ...
@typing.overload
def log2(arg0: float3) -> float3:
    ...
@typing.overload
def log2(arg0: float4) -> float4:
    ...
@typing.overload
def log2(arg0: float2x2) -> float2x2:
    ...
@typing.overload
def log2(arg0: float2x3) -> float2x3:
    ...
@typing.overload
def log2(arg0: float2x4) -> float2x4:
    ...
@typing.overload
def log2(arg0: float3x2) -> float3x2:
    ...
@typing.overload
def log2(arg0: float3x3) -> float3x3:
    ...
@typing.overload
def log2(arg0: float3x4) -> float3x4:
    ...
@typing.overload
def log2(arg0: float4x2) -> float4x2:
    ...
@typing.overload
def log2(arg0: float4x3) -> float4x3:
    ...
@typing.overload
def log2(arg0: float4x4) -> float4x4:
    ...
@typing.overload
def make_bool2(arg0: uint2) -> bool2:
    ...
@typing.overload
def make_bool2(arg0: int2) -> bool2:
    ...
@typing.overload
def make_bool2(arg0: float2) -> bool2:
    ...
@typing.overload
def make_bool2(arg0: bool) -> bool2:
    ...
@typing.overload
def make_bool2(arg0: bool, arg1: bool) -> bool2:
    ...
@typing.overload
def make_bool2(arg0: bool2) -> bool2:
    ...
@typing.overload
def make_bool3(arg0: uint3) -> bool3:
    ...
@typing.overload
def make_bool3(arg0: int3) -> bool3:
    ...
@typing.overload
def make_bool3(arg0: float3) -> bool3:
    ...
@typing.overload
def make_bool3(arg0: bool) -> bool3:
    ...
@typing.overload
def make_bool3(arg0: bool, arg1: bool, arg2: bool) -> bool3:
    ...
@typing.overload
def make_bool3(arg0: bool3) -> bool3:
    ...
@typing.overload
def make_bool4(arg0: uint4) -> bool4:
    ...
@typing.overload
def make_bool4(arg0: int4) -> bool4:
    ...
@typing.overload
def make_bool4(arg0: float4) -> bool4:
    ...
@typing.overload
def make_bool4(arg0: bool) -> bool4:
    ...
@typing.overload
def make_bool4(arg0: bool, arg1: bool, arg2: bool, arg3: bool) -> bool4:
    ...
@typing.overload
def make_bool4(arg0: bool4) -> bool4:
    ...
@typing.overload
def make_float2(*args, **kwargs) -> float2:
    ...
@typing.overload
def make_float2(*args, **kwargs) -> float2:
    ...
@typing.overload
def make_float2(*args, **kwargs) -> float2:
    ...
@typing.overload
def make_float2(arg0: float) -> float2:
    ...
@typing.overload
def make_float2(arg0: float, arg1: float) -> float2:
    ...
@typing.overload
def make_float2(arg0: float2) -> float2:
    ...
@typing.overload
def make_float2x2(arg0: float) -> float2x2:
    ...
@typing.overload
def make_float2x2(arg0: float2x2) -> float2x2:
    ...
@typing.overload
def make_float2x2(arg0: typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(4)]) -> float2x2:
    ...
@typing.overload
def make_float2x2(arg0: typing.Annotated[list[float2], pybind11_stubgen.typing_ext.FixedSize(2)]) -> float2x2:
    ...
@typing.overload
def make_float2x2(arg0: typing.Annotated[list[typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(2)]], pybind11_stubgen.typing_ext.FixedSize(2)]) -> float2x2:
    ...
@typing.overload
def make_float2x3(arg0: float) -> float2x3:
    ...
@typing.overload
def make_float2x3(arg0: float2x3) -> float2x3:
    ...
@typing.overload
def make_float2x3(arg0: typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(6)]) -> float2x3:
    ...
@typing.overload
def make_float2x3(arg0: typing.Annotated[list[float3], pybind11_stubgen.typing_ext.FixedSize(2)]) -> float2x3:
    ...
@typing.overload
def make_float2x3(arg0: typing.Annotated[list[typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(3)]], pybind11_stubgen.typing_ext.FixedSize(2)]) -> float2x3:
    ...
@typing.overload
def make_float2x4(arg0: float) -> float2x4:
    ...
@typing.overload
def make_float2x4(arg0: float2x4) -> float2x4:
    ...
@typing.overload
def make_float2x4(arg0: typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(8)]) -> float2x4:
    ...
@typing.overload
def make_float2x4(arg0: typing.Annotated[list[float4], pybind11_stubgen.typing_ext.FixedSize(2)]) -> float2x4:
    ...
@typing.overload
def make_float2x4(arg0: typing.Annotated[list[typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(4)]], pybind11_stubgen.typing_ext.FixedSize(2)]) -> float2x4:
    ...
@typing.overload
def make_float3(*args, **kwargs) -> float3:
    ...
@typing.overload
def make_float3(*args, **kwargs) -> float3:
    ...
@typing.overload
def make_float3(*args, **kwargs) -> float3:
    ...
@typing.overload
def make_float3(arg0: float) -> float3:
    ...
@typing.overload
def make_float3(arg0: float, arg1: float, arg2: float) -> float3:
    ...
@typing.overload
def make_float3(arg0: float3) -> float3:
    ...
@typing.overload
def make_float3x2(arg0: float) -> float3x2:
    ...
@typing.overload
def make_float3x2(arg0: float3x2) -> float3x2:
    ...
@typing.overload
def make_float3x2(arg0: typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(6)]) -> float3x2:
    ...
@typing.overload
def make_float3x2(arg0: typing.Annotated[list[float2], pybind11_stubgen.typing_ext.FixedSize(3)]) -> float3x2:
    ...
@typing.overload
def make_float3x2(arg0: typing.Annotated[list[typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(2)]], pybind11_stubgen.typing_ext.FixedSize(3)]) -> float3x2:
    ...
@typing.overload
def make_float3x3(arg0: float) -> float3x3:
    ...
@typing.overload
def make_float3x3(arg0: float3x3) -> float3x3:
    ...
@typing.overload
def make_float3x3(arg0: typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(9)]) -> float3x3:
    ...
@typing.overload
def make_float3x3(arg0: typing.Annotated[list[float3], pybind11_stubgen.typing_ext.FixedSize(3)]) -> float3x3:
    ...
@typing.overload
def make_float3x3(arg0: typing.Annotated[list[typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(3)]], pybind11_stubgen.typing_ext.FixedSize(3)]) -> float3x3:
    ...
@typing.overload
def make_float3x4(arg0: float) -> float3x4:
    ...
@typing.overload
def make_float3x4(arg0: float3x4) -> float3x4:
    ...
@typing.overload
def make_float3x4(arg0: typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(12)]) -> float3x4:
    ...
@typing.overload
def make_float3x4(arg0: typing.Annotated[list[float4], pybind11_stubgen.typing_ext.FixedSize(3)]) -> float3x4:
    ...
@typing.overload
def make_float3x4(arg0: typing.Annotated[list[typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(4)]], pybind11_stubgen.typing_ext.FixedSize(3)]) -> float3x4:
    ...
@typing.overload
def make_float4(*args, **kwargs) -> float4:
    ...
@typing.overload
def make_float4(*args, **kwargs) -> float4:
    ...
@typing.overload
def make_float4(*args, **kwargs) -> float4:
    ...
@typing.overload
def make_float4(arg0: float) -> float4:
    ...
@typing.overload
def make_float4(arg0: float, arg1: float, arg2: float, arg3: float) -> float4:
    ...
@typing.overload
def make_float4(arg0: float4) -> float4:
    ...
@typing.overload
def make_float4x2(arg0: float) -> float4x2:
    ...
@typing.overload
def make_float4x2(arg0: float4x2) -> float4x2:
    ...
@typing.overload
def make_float4x2(arg0: typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(8)]) -> float4x2:
    ...
@typing.overload
def make_float4x2(arg0: typing.Annotated[list[float2], pybind11_stubgen.typing_ext.FixedSize(4)]) -> float4x2:
    ...
@typing.overload
def make_float4x2(arg0: typing.Annotated[list[typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(2)]], pybind11_stubgen.typing_ext.FixedSize(4)]) -> float4x2:
    ...
@typing.overload
def make_float4x3(arg0: float) -> float4x3:
    ...
@typing.overload
def make_float4x3(arg0: float4x3) -> float4x3:
    ...
@typing.overload
def make_float4x3(arg0: typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(12)]) -> float4x3:
    ...
@typing.overload
def make_float4x3(arg0: typing.Annotated[list[float3], pybind11_stubgen.typing_ext.FixedSize(4)]) -> float4x3:
    ...
@typing.overload
def make_float4x3(arg0: typing.Annotated[list[typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(3)]], pybind11_stubgen.typing_ext.FixedSize(4)]) -> float4x3:
    ...
@typing.overload
def make_float4x4(arg0: float) -> float4x4:
    ...
@typing.overload
def make_float4x4(arg0: float4x4) -> float4x4:
    ...
@typing.overload
def make_float4x4(arg0: typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(16)]) -> float4x4:
    ...
@typing.overload
def make_float4x4(arg0: typing.Annotated[list[float4], pybind11_stubgen.typing_ext.FixedSize(4)]) -> float4x4:
    ...
@typing.overload
def make_float4x4(arg0: typing.Annotated[list[typing.Annotated[list[float], pybind11_stubgen.typing_ext.FixedSize(4)]], pybind11_stubgen.typing_ext.FixedSize(4)]) -> float4x4:
    ...
@typing.overload
def make_int2(arg0: uint2) -> int2:
    ...
@typing.overload
def make_int2(arg0: float2) -> int2:
    ...
@typing.overload
def make_int2(*args, **kwargs) -> int2:
    ...
@typing.overload
def make_int2(arg0: int) -> int2:
    ...
@typing.overload
def make_int2(arg0: int, arg1: int) -> int2:
    ...
@typing.overload
def make_int2(arg0: int2) -> int2:
    ...
@typing.overload
def make_int3(arg0: uint3) -> int3:
    ...
@typing.overload
def make_int3(arg0: float3) -> int3:
    ...
@typing.overload
def make_int3(*args, **kwargs) -> int3:
    ...
@typing.overload
def make_int3(arg0: int) -> int3:
    ...
@typing.overload
def make_int3(arg0: int, arg1: int, arg2: int) -> int3:
    ...
@typing.overload
def make_int3(arg0: int3) -> int3:
    ...
@typing.overload
def make_int4(arg0: uint4) -> int4:
    ...
@typing.overload
def make_int4(arg0: float4) -> int4:
    ...
@typing.overload
def make_int4(*args, **kwargs) -> int4:
    ...
@typing.overload
def make_int4(arg0: int) -> int4:
    ...
@typing.overload
def make_int4(arg0: int, arg1: int, arg2: int, arg3: int) -> int4:
    ...
@typing.overload
def make_int4(arg0: int4) -> int4:
    ...
@typing.overload
def make_uint2(*args, **kwargs) -> uint2:
    ...
@typing.overload
def make_uint2(arg0: float2) -> uint2:
    ...
@typing.overload
def make_uint2(*args, **kwargs) -> uint2:
    ...
@typing.overload
def make_uint2(arg0: int) -> uint2:
    ...
@typing.overload
def make_uint2(arg0: int, arg1: int) -> uint2:
    ...
@typing.overload
def make_uint2(arg0: uint2) -> uint2:
    ...
@typing.overload
def make_uint3(*args, **kwargs) -> uint3:
    ...
@typing.overload
def make_uint3(arg0: float3) -> uint3:
    ...
@typing.overload
def make_uint3(*args, **kwargs) -> uint3:
    ...
@typing.overload
def make_uint3(arg0: int) -> uint3:
    ...
@typing.overload
def make_uint3(arg0: int, arg1: int, arg2: int) -> uint3:
    ...
@typing.overload
def make_uint3(arg0: uint3) -> uint3:
    ...
@typing.overload
def make_uint4(*args, **kwargs) -> uint4:
    ...
@typing.overload
def make_uint4(arg0: float4) -> uint4:
    ...
@typing.overload
def make_uint4(*args, **kwargs) -> uint4:
    ...
@typing.overload
def make_uint4(arg0: int) -> uint4:
    ...
@typing.overload
def make_uint4(arg0: int, arg1: int, arg2: int, arg3: int) -> uint4:
    ...
@typing.overload
def make_uint4(arg0: uint4) -> uint4:
    ...
@typing.overload
def max(arg0: float2, arg1: float2) -> float2:
    ...
@typing.overload
def max(arg0: uint2, arg1: uint2) -> uint2:
    ...
@typing.overload
def max(arg0: int2, arg1: int2) -> int2:
    ...
@typing.overload
def max(arg0: float3, arg1: float3) -> float3:
    ...
@typing.overload
def max(arg0: uint3, arg1: uint3) -> uint3:
    ...
@typing.overload
def max(arg0: int3, arg1: int3) -> int3:
    ...
@typing.overload
def max(arg0: float4, arg1: float4) -> float4:
    ...
@typing.overload
def max(arg0: uint4, arg1: uint4) -> uint4:
    ...
@typing.overload
def max(arg0: int4, arg1: int4) -> int4:
    ...
@typing.overload
def min(arg0: float2, arg1: float2) -> float2:
    ...
@typing.overload
def min(arg0: uint2, arg1: uint2) -> uint2:
    ...
@typing.overload
def min(arg0: int2, arg1: int2) -> int2:
    ...
@typing.overload
def min(arg0: float3, arg1: float3) -> float3:
    ...
@typing.overload
def min(arg0: uint3, arg1: uint3) -> uint3:
    ...
@typing.overload
def min(arg0: int3, arg1: int3) -> int3:
    ...
@typing.overload
def min(arg0: float4, arg1: float4) -> float4:
    ...
@typing.overload
def min(arg0: uint4, arg1: uint4) -> uint4:
    ...
@typing.overload
def min(arg0: int4, arg1: int4) -> int4:
    ...
@typing.overload
def none(arg0: bool2) -> bool:
    ...
@typing.overload
def none(arg0: bool3) -> bool:
    ...
@typing.overload
def none(arg0: bool4) -> bool:
    ...
@typing.overload
def normalize(arg0: float2) -> float2:
    ...
@typing.overload
def normalize(arg0: float3) -> float3:
    ...
@typing.overload
def normalize(arg0: float4) -> float4:
    ...
@typing.overload
def pow(arg0: float2, arg1: float2) -> float2:
    ...
@typing.overload
def pow(arg0: float3, arg1: float3) -> float3:
    ...
@typing.overload
def pow(arg0: float4, arg1: float4) -> float4:
    ...
@typing.overload
def radians(arg0: float2) -> float2:
    ...
@typing.overload
def radians(arg0: float3) -> float3:
    ...
@typing.overload
def radians(arg0: float4) -> float4:
    ...
@typing.overload
def radians(arg0: float2x2) -> float2x2:
    ...
@typing.overload
def radians(arg0: float2x3) -> float2x3:
    ...
@typing.overload
def radians(arg0: float2x4) -> float2x4:
    ...
@typing.overload
def radians(arg0: float3x2) -> float3x2:
    ...
@typing.overload
def radians(arg0: float3x3) -> float3x3:
    ...
@typing.overload
def radians(arg0: float3x4) -> float3x4:
    ...
@typing.overload
def radians(arg0: float4x2) -> float4x2:
    ...
@typing.overload
def radians(arg0: float4x3) -> float4x3:
    ...
@typing.overload
def radians(arg0: float4x4) -> float4x4:
    ...
@typing.overload
def rcp(arg0: float2) -> float2:
    ...
@typing.overload
def rcp(arg0: float3) -> float3:
    ...
@typing.overload
def rcp(arg0: float4) -> float4:
    ...
@typing.overload
def rcp(arg0: float2x2) -> float2x2:
    ...
@typing.overload
def rcp(arg0: float2x3) -> float2x3:
    ...
@typing.overload
def rcp(arg0: float2x4) -> float2x4:
    ...
@typing.overload
def rcp(arg0: float3x2) -> float3x2:
    ...
@typing.overload
def rcp(arg0: float3x3) -> float3x3:
    ...
@typing.overload
def rcp(arg0: float3x4) -> float3x4:
    ...
@typing.overload
def rcp(arg0: float4x2) -> float4x2:
    ...
@typing.overload
def rcp(arg0: float4x3) -> float4x3:
    ...
@typing.overload
def rcp(arg0: float4x4) -> float4x4:
    ...
@typing.overload
def round(arg0: float2) -> float2:
    ...
@typing.overload
def round(arg0: float3) -> float3:
    ...
@typing.overload
def round(arg0: float4) -> float4:
    ...
@typing.overload
def round(arg0: float2x2) -> float2x2:
    ...
@typing.overload
def round(arg0: float2x3) -> float2x3:
    ...
@typing.overload
def round(arg0: float2x4) -> float2x4:
    ...
@typing.overload
def round(arg0: float3x2) -> float3x2:
    ...
@typing.overload
def round(arg0: float3x3) -> float3x3:
    ...
@typing.overload
def round(arg0: float3x4) -> float3x4:
    ...
@typing.overload
def round(arg0: float4x2) -> float4x2:
    ...
@typing.overload
def round(arg0: float4x3) -> float4x3:
    ...
@typing.overload
def round(arg0: float4x4) -> float4x4:
    ...
@typing.overload
def sign(arg0: float2) -> float2:
    ...
@typing.overload
def sign(arg0: int2) -> int2:
    ...
@typing.overload
def sign(arg0: float3) -> float3:
    ...
@typing.overload
def sign(arg0: int3) -> int3:
    ...
@typing.overload
def sign(arg0: float4) -> float4:
    ...
@typing.overload
def sign(arg0: int4) -> int4:
    ...
@typing.overload
def sign(arg0: float2x2) -> float2x2:
    ...
@typing.overload
def sign(arg0: float2x3) -> float2x3:
    ...
@typing.overload
def sign(arg0: float2x4) -> float2x4:
    ...
@typing.overload
def sign(arg0: float3x2) -> float3x2:
    ...
@typing.overload
def sign(arg0: float3x3) -> float3x3:
    ...
@typing.overload
def sign(arg0: float3x4) -> float3x4:
    ...
@typing.overload
def sign(arg0: float4x2) -> float4x2:
    ...
@typing.overload
def sign(arg0: float4x3) -> float4x3:
    ...
@typing.overload
def sign(arg0: float4x4) -> float4x4:
    ...
@typing.overload
def sin(arg0: float2) -> float2:
    ...
@typing.overload
def sin(arg0: float3) -> float3:
    ...
@typing.overload
def sin(arg0: float4) -> float4:
    ...
@typing.overload
def sin(arg0: float2x2) -> float2x2:
    ...
@typing.overload
def sin(arg0: float2x3) -> float2x3:
    ...
@typing.overload
def sin(arg0: float2x4) -> float2x4:
    ...
@typing.overload
def sin(arg0: float3x2) -> float3x2:
    ...
@typing.overload
def sin(arg0: float3x3) -> float3x3:
    ...
@typing.overload
def sin(arg0: float3x4) -> float3x4:
    ...
@typing.overload
def sin(arg0: float4x2) -> float4x2:
    ...
@typing.overload
def sin(arg0: float4x3) -> float4x3:
    ...
@typing.overload
def sin(arg0: float4x4) -> float4x4:
    ...
@typing.overload
def sinh(arg0: float2) -> float2:
    ...
@typing.overload
def sinh(arg0: float3) -> float3:
    ...
@typing.overload
def sinh(arg0: float4) -> float4:
    ...
@typing.overload
def sinh(arg0: float2x2) -> float2x2:
    ...
@typing.overload
def sinh(arg0: float2x3) -> float2x3:
    ...
@typing.overload
def sinh(arg0: float2x4) -> float2x4:
    ...
@typing.overload
def sinh(arg0: float3x2) -> float3x2:
    ...
@typing.overload
def sinh(arg0: float3x3) -> float3x3:
    ...
@typing.overload
def sinh(arg0: float3x4) -> float3x4:
    ...
@typing.overload
def sinh(arg0: float4x2) -> float4x2:
    ...
@typing.overload
def sinh(arg0: float4x3) -> float4x3:
    ...
@typing.overload
def sinh(arg0: float4x4) -> float4x4:
    ...
@typing.overload
def sqr(arg0: float2) -> float2:
    ...
@typing.overload
def sqr(arg0: float3) -> float3:
    ...
@typing.overload
def sqr(arg0: float4) -> float4:
    ...
@typing.overload
def sqr(arg0: float2x2) -> float2x2:
    ...
@typing.overload
def sqr(arg0: float2x3) -> float2x3:
    ...
@typing.overload
def sqr(arg0: float2x4) -> float2x4:
    ...
@typing.overload
def sqr(arg0: float3x2) -> float3x2:
    ...
@typing.overload
def sqr(arg0: float3x3) -> float3x3:
    ...
@typing.overload
def sqr(arg0: float3x4) -> float3x4:
    ...
@typing.overload
def sqr(arg0: float4x2) -> float4x2:
    ...
@typing.overload
def sqr(arg0: float4x3) -> float4x3:
    ...
@typing.overload
def sqr(arg0: float4x4) -> float4x4:
    ...
@typing.overload
def sqrt(arg0: float2) -> float2:
    ...
@typing.overload
def sqrt(arg0: float3) -> float3:
    ...
@typing.overload
def sqrt(arg0: float4) -> float4:
    ...
@typing.overload
def sqrt(arg0: float2x2) -> float2x2:
    ...
@typing.overload
def sqrt(arg0: float2x3) -> float2x3:
    ...
@typing.overload
def sqrt(arg0: float2x4) -> float2x4:
    ...
@typing.overload
def sqrt(arg0: float3x2) -> float3x2:
    ...
@typing.overload
def sqrt(arg0: float3x3) -> float3x3:
    ...
@typing.overload
def sqrt(arg0: float3x4) -> float3x4:
    ...
@typing.overload
def sqrt(arg0: float4x2) -> float4x2:
    ...
@typing.overload
def sqrt(arg0: float4x3) -> float4x3:
    ...
@typing.overload
def sqrt(arg0: float4x4) -> float4x4:
    ...
@typing.overload
def tan(arg0: float2) -> float2:
    ...
@typing.overload
def tan(arg0: float3) -> float3:
    ...
@typing.overload
def tan(arg0: float4) -> float4:
    ...
@typing.overload
def tan(arg0: float2x2) -> float2x2:
    ...
@typing.overload
def tan(arg0: float2x3) -> float2x3:
    ...
@typing.overload
def tan(arg0: float2x4) -> float2x4:
    ...
@typing.overload
def tan(arg0: float3x2) -> float3x2:
    ...
@typing.overload
def tan(arg0: float3x3) -> float3x3:
    ...
@typing.overload
def tan(arg0: float3x4) -> float3x4:
    ...
@typing.overload
def tan(arg0: float4x2) -> float4x2:
    ...
@typing.overload
def tan(arg0: float4x3) -> float4x3:
    ...
@typing.overload
def tan(arg0: float4x4) -> float4x4:
    ...
@typing.overload
def tanh(arg0: float2) -> float2:
    ...
@typing.overload
def tanh(arg0: float3) -> float3:
    ...
@typing.overload
def tanh(arg0: float4) -> float4:
    ...
@typing.overload
def tanh(arg0: float2x2) -> float2x2:
    ...
@typing.overload
def tanh(arg0: float2x3) -> float2x3:
    ...
@typing.overload
def tanh(arg0: float2x4) -> float2x4:
    ...
@typing.overload
def tanh(arg0: float3x2) -> float3x2:
    ...
@typing.overload
def tanh(arg0: float3x3) -> float3x3:
    ...
@typing.overload
def tanh(arg0: float3x4) -> float3x4:
    ...
@typing.overload
def tanh(arg0: float4x2) -> float4x2:
    ...
@typing.overload
def tanh(arg0: float4x3) -> float4x3:
    ...
@typing.overload
def tanh(arg0: float4x4) -> float4x4:
    ...
@typing.overload
def transpose(arg0: float2x2) -> float2x2:
    ...
@typing.overload
def transpose(arg0: float3x3) -> float3x3:
    ...
@typing.overload
def transpose(arg0: float4x4) -> float4x4:
    ...
