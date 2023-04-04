# -*- coding:utf-8 -*-
from os.path import realpath, dirname

def generate(file, dim):
    input = ["x", "y", "z", "w"]
    if dim == 1:
        for i, x in enumerate(input):
            str = f"case 0x{i}: return _node->value(ctx).{x}();"
            print(str, file=file)
    elif dim == 2:
        for i, x in enumerate(input):
            for j, y in enumerate(input):
                str = f"case 0x{i}{j}: return _node->value(ctx).{x}{y}();"
                print(str, file=file)
    elif dim == 3:
        for i, x in enumerate(input):
            for j, y in enumerate(input):
                for k, z in enumerate(input):
                    str = f"case 0x{i}{j}{k}: return _node->value(ctx).{x}{y}{z}();"
                    print(str, file=file)
    else:
        for i, x in enumerate(input):
            for j, y in enumerate(input):
                for k, z in enumerate(input):
                    for l, w in enumerate(input):
                        str = f"case 0x{i}{j}{k}{l}: return _node->value(ctx).{x}{y}{z}{w}();"
                        print(str, file=file)
                        
def generate_new(file, dim):
    input = ["x", "y", "z", "w"]
    if dim == 1:
        for i, x in enumerate(input):
            str = f"case 0x{i}: return _node->evaluate(ctx, da).{x}();"
            print(str, file=file)
    elif dim == 2:
        for i, x in enumerate(input):
            for j, y in enumerate(input):
                str = f"case 0x{i}{j}: return _node->evaluate(ctx, da).{x}{y}();"
                print(str, file=file)
    elif dim == 3:
        for i, x in enumerate(input):
            for j, y in enumerate(input):
                for k, z in enumerate(input):
                    str = f"case 0x{i}{j}{k}: return _node->evaluate(ctx, da).{x}{y}{z}();"
                    print(str, file=file)
    else:
        for i, x in enumerate(input):
            for j, y in enumerate(input):
                for k, z in enumerate(input):
                    for l, w in enumerate(input):
                        str = f"case 0x{i}{j}{k}{l}: return _node->evaluate(ctx, da).{x}{y}{z}{w}();"
                        print(str, file=file)

if __name__ == "__main__":
    base = dirname(realpath(__file__))
    for dim in range(1, 5):
        with open(f"{base}/slot_swizzle_{dim}.inl.h", "w") as file:
            generate(file, dim)
            
    base = dirname(realpath(__file__))
    for dim in range(1, 5):
        with open(f"{base}/slot_swizzle{dim}.inl.h", "w") as file:
            generate_new(file, dim)