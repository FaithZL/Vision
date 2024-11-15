import ast
import inspect
from ast import *

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
    
    def visit_Load(self, node: Load):
        pass
    def visit_Store(self, node: Store):
        pass
       
    def visit_IfExp(self, node: IfExp):
        pass
    
    def visit_Call(self, node: Call):
        pass
    
    def visit_Sub(self, node: Sub):
        pass
    
    def visit_And(self, node: And):
        pass
    def visit_Or(self, node: Or):
        pass
    def visit_Add(self, node: Add):
        pass
    def visit_BitAnd(self, node: BitAnd):
        pass
    def visit_BitOr(self, node: BitOr):
        pass
    def visit_BitXor(self, node: BitXor):
        pass
    def visit_Div(self, node: Div):
        pass
    def visit_FloorDiv(self, node: FloorDiv):
        pass
    def visit_LShift(self, node: LShift):
        pass
    def visit_Mod(self, node: Mod):
        pass
    def visit_Mult(self, node: Mult):
        pass
    def visit_Pow(self, node: Pow):
        pass
    def visit_RShift(self, node: RShift):
        pass
    def visit_UAdd(self, node: UAdd):
        pass
    def visit_USub(self, node: USub):
        pass

    def visit_Eq(self, node: Eq):
        pass
    def visit_Gt(self, node: Gt):
        pass
    def visit_GtE(self, node: GtE):
        pass
    def visit_Lt(self, node: Lt):
        pass
    def visit_LtE(self, node: LtE):
        pass
    def visit_NotEq(self, node: NotEq):
        pass
    def visit_arguments(self, node: arguments):
        pass
    def visit_AugLoad(self, node: AugLoad):
        pass
    def visit_AugStore(self, node: AugStore):
        pass

class Test:
    a : int

def func(x : int):
    t = Test()
    t.a = x + 5
    return x + x * 2;

def example_function(x: int):
    x += 5;
    a:int = x + x
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