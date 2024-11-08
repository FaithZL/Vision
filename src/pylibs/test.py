import cpplibs
import os
from cpplibs import ocapi
from cpplibs.ocapi import *
from cpplibs import vsapi
from cpplibs.vsapi import *
import numpy as np

f2 = cpplibs.ocapi.float2(1,2)
print(f2)
f2[0] = 3.88999
print(f2.x)
print(f2.xx)
f2.xy = cpplibs.ocapi.float2(5,6)
print(f2.xx)
print(f2.xy)
print(f2.xyx * f2.xyx)

f3 = f2.xyx

print(cpplibs.ocapi.lerp(f3 * 0 , f3 / 2 , f3))
print(cpplibs.ocapi.make_bool3(f3))
print(as_float3(ocapi.as_uint3(f3)))

m = float2x2()
m[0] = make_float2(1,6)

print(m)
print(float3([1,22,0]))
print(float2x2([float2(1,2),float2(3,4)]))
print(inverse(inverse(float2x2([float2(1,2),float2(3,4)]))))
print(make_float2x2([(1,2),(3,5)]) * 2)
print(make_float2x2([(1,2),(3,5)]) * make_float2x2([(1,2),(3,5)]))
print(make_float2x2([(1,2),(3,5)]) * make_float2x2([(1,2),(3,5)])[1])
print(make_float2x2(make_float2x2([(1,2),(3,5)])).clone())
cpplibs.init_context("cuda")





print("wocao")
# t = Type.from_desc(float2().desc())
# print(t.description())
# print(t.name())
# print(as_float(as_uint(2.0)))
# print(as_float(as_uint(2.0)))
# print(as_float(as_uint(2.0)))
buffer = Bufferfloat2.create(2)
# print(buffer.size())
buffer.upload([float2(3, 4), float2(10, 11)])

af = Arrayfloat2()
af.resize(5)

print(af)
# af.push_back(float2(5.5, 10000))
# af.push_back(float2(5.5, 999))
# print(af[0])
# print(af)
# af.clear()
# print(af)
# # print(Arrayfloat())

arr = np.array([[1.0, 5.5], [5,9]], dtype=np.float32)
# arr = [1.0, 5.5]

buffer.download(arr)
print(arr)
print(buffer.download())

# a = arr[1]

# # buffer.download()

# # print(a2)

# # lst = []
# # vsapi.test(lst)

# print(lst)
# th = TriangleHit()
# th.bary = float3(1,2,3).xz
# print(Ray(float3(1,2,3), float3(1,2,5)).origin)
buffer = None

import traceback

print("------------------------")

import ast
import inspect

class MyVisitor(ast.NodeVisitor):
    def visit_FunctionDef(self, node):
        print(f"Function name: {node.name}")
        self.generic_visit(node)  # 继续遍历子节点

    def visit_Return(self, node):
        print("Return statement found")
        self.generic_visit(node)  # 继续遍历子节点

    def visit_BinOp(self, node):
        print("Binary operation found")
        self.generic_visit(node)  # 继续遍历子节点

    def visit_For(self, node):
        print(node.target.id)
        self.generic_visit(node)

    def visit_Name(self, node):
        print(f"Variable name: {node.id}")
        self.generic_visit(node)  # 继续遍历子节点

    def visit_Constant(self, node):
        print(f"Constant value: {node.value}")
        self.generic_visit(node)  # 继续遍历子节点



# 定义一个示例函数
def example_function(x: int):
    # x = v.xxyz
    return x * 2

example_function(5)
# # 获取函数的源代码
# source_code = inspect.getsource(example_function)

# # 解析源代码为 AST
# parsed_ast = ast.parse(source_code)

# visitor = MyVisitor()
# visitor.visit(parsed_ast)

# # 打印 AST
# print(ast.dump(parsed_ast, indent=4))

# d = ocapi.device()


def on_mouse(*arg):
    print(*arg)
    
# fn = os.getcwd() + "/res/render_scene/cbox/dispersion-hero-2000.exr"
fn = os.getcwd() + "/res/render_scene/cbox/dispersion-hero.png"

image = Image.load(fn, ColorSpace.LINEAR)

iaar = image.as_uchar_array_t()

print(iaar.shape)
res = 1024

w = Window.create(res, res)

af = Arrayfloat4()

af.resize(res * res, float4(0,0,1,1))

arr = np.ones(res * res * 4, dtype=np.float32)

w.set_clear_color(float4(1,0,0,1))

# print(arr)

w.set_mouse_callback(on_mouse)

def func(t):
    w.set_background(iaar)
    # w.set_should_close()
    # w.set_background(arr)
    pass

w.run(func)

ocapi.exit()