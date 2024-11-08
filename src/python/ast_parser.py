import ast
import inspect

class MyVisitor(ast.NodeVisitor):
    def visit_FunctionDef(self, node):
        print(f"Function name: {node.name}")
        self.generic_visit(node)  

    def visit_Return(self, node):
        print("Return statement found")
        self.generic_visit(node)  

    def visit_BinOp(self, node):
        print("Binary operation found")
        self.generic_visit(node)  

    def visit_For(self, node):
        print(node.target.id)
        self.generic_visit(node)

    def visit_Name(self, node):
        print(f"Variable name: {node.id}")
        self.generic_visit(node)  

    def visit_Constant(self, node):
        print(f"Constant value: {node.value}")
        self.generic_visit(node)  



def example_function(x: int):
    return x * 2


source_code = inspect.getsource(example_function)

parsed_ast = ast.parse(source_code)

visitor = MyVisitor()
visitor.visit(parsed_ast)

print(ast.dump(parsed_ast, indent=4))