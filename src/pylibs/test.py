import vision
import os
from vision import ocapi
from vision.ocapi import *
from vision import vsapi
from vision.vsapi import *
import numpy as np

f2 = vision.ocapi.float2(1,2)
print(f2)
f2[0] = 3.88999
print(f2.x)
print(f2.xx)
f2.xy = vision.ocapi.float2(5,6)
print(f2.xx)
print(f2.xy)
print(f2.xyx * f2.xyx)

f3 = f2.xyx

print(vision.ocapi.lerp(f3 * 0 , f3 / 2 , f3))
print(vision.ocapi.make_bool3(f3))
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
vision.init_context("cuda")





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


# def on_mouse(*arg):
#     print(*arg)
    


# w = Window.create(500, 500)

# # w.set_clear_color(float4(1,1,1,1))

# w.set_mouse_callback(on_mouse)

# def func(t):
#     pass


# w.run(func)

# ocapi.exit()