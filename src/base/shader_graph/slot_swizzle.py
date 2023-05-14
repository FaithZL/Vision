# -*- coding:utf-8 -*-
from os.path import realpath, dirname

def generate(file, dim):
    input = ["x", "y", "z", "w"]
    if dim == 1:
        for i, x in enumerate(input):
            str = f"case 0x{i}: return _node->evaluate(ctx, swl).{x}();"
            print(str, file=file)
    elif dim == 2:
        for i, x in enumerate(input):
            for j, y in enumerate(input):
                str = f"case 0x{i}{j}: return _node->evaluate(ctx, swl).{x}{y}();"
                print(str, file=file)
    elif dim == 3:
        for i, x in enumerate(input):
            for j, y in enumerate(input):
                for k, z in enumerate(input):
                    str = f"case 0x{i}{j}{k}: return _node->evaluate(ctx, swl).{x}{y}{z}();"
                    print(str, file=file)
    else:
        for i, x in enumerate(input):
            for j, y in enumerate(input):
                for k, z in enumerate(input):
                    for l, w in enumerate(input):
                        str = f"case 0x{i}{j}{k}{l}: return _node->evaluate(ctx, swl).{x}{y}{z}{w}();"
                        print(str, file=file)

def generate_average(file, dim):
    input = ["x", "y", "z", "w"]
    if dim == 1:
        for i, x in enumerate(input):
            str = f"case 0x{i}: return ocarina::vector<float>{{_node->average()[{i}]}};"
            print(str, file=file)
    elif dim == 2:
        for i, x in enumerate(input):
            for j, y in enumerate(input):
                str = f"case 0x{i}{j}: return ocarina::vector<float>{{_node->average()[{i}],_node->average()[{j}]}};"
                print(str, file=file)
    elif dim == 3:
        for i, x in enumerate(input):
            for j, y in enumerate(input):
                for k, z in enumerate(input):
                    str = f"case 0x{i}{j}{k}: return ocarina::vector<float>{{_node->average()[{i}], _node->average()[{j}], _node->average()[{k}]}};"
                    print(str, file=file)
    else:
        for i, x in enumerate(input):
            for j, y in enumerate(input):
                for k, z in enumerate(input):
                    for l, w in enumerate(input):
                        str = f"case 0x{i}{j}{k}{l}: return ocarina::vector<float>{{_node->average()[{i}], _node->average()[{j}], _node->average()[{k}], _node->average()[{l}]}};"
                        print(str, file=file)

if __name__ == "__main__":
    base = dirname(realpath(__file__))
    for dim in range(1, 5):
        with open(f"{base}/slot_swizzle_{dim}.inl.h", "w") as file:
            generate(file, dim)
    
    for dim in range(1, 5):
        with open(f"{base}/slot_average_swizzle_{dim}.inl.h", "w") as file:
            generate_average(file, dim)
            