import ast
import inspect

class OcarinaVisitor(ast.NodeVisitor):
    def visit_FunctionDef(self, node):
        print(f"Function name: {node.name}")
        self.generic_visit(node)  

    def visit_Return(self, node):
        print("Return statement found")
        self.generic_visit(node)  

    def visit_Expression(self, node):
        print("visit_Expression found")
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

class Test:
    a : int

def func(x : int):
    t = Test()
    t.a = x + 5
    return x + x * 2;

def example_function(x: int):
    x += 5;
    a = x + x
    if x > 10:
        a = 9
    elif x < 6:
        a = x
    else:
        a = x + a
        
    for i in range(0, 10, 2):
        x += i
    
    arr = [1,2,4]
    
    arr2 = [i for i in range(x)]
    
    x += func(arr[1])
    
    x = x << 9

    a = 10 if x > 9 else 15

    t = Test()
    t.a = x + 5
    
    while(x > 10):
        x -= 1
    
    b = -x > 0
    
    return x * 2


source_code = inspect.getsource(example_function)

parsed_ast = ast.parse(source_code)

visitor = OcarinaVisitor()
visitor.visit(parsed_ast)

print(ast.dump(parsed_ast, indent=4))