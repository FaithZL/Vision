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


# 获取函数的源代码
source_code = inspect.getsource(example_function)

# 解析源代码为 AST
parsed_ast = ast.parse(source_code)

visitor = MyVisitor()
visitor.visit(parsed_ast)

# 打印 AST
print(ast.dump(parsed_ast, indent=4))