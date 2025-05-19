# -*- coding:utf-8 -*-
from os.path import realpath, dirname

def generate(file, dim):
    input = ["x", "y", "z", "w"]
    if dim == 1:
        for i, x in enumerate(input):
            str = f"case 0x{i}: return node_->evaluate(ctx, swl).array.{x}();"
            print(str, file=file)
    elif dim == 2:
        for i, x in enumerate(input):
            for j, y in enumerate(input):
                str = f"case 0x{i}{j}: return node_->evaluate(ctx, swl).array.{x}{y}();"
                print(str, file=file)
    elif dim == 3:
        for i, x in enumerate(input):
            for j, y in enumerate(input):
                for k, z in enumerate(input):
                    str = f"case 0x{i}{j}{k}: return node_->evaluate(ctx, swl).array.{x}{y}{z}();"
                    print(str, file=file)
    else:
        for i, x in enumerate(input):
            for j, y in enumerate(input):
                for k, z in enumerate(input):
                    for l, w in enumerate(input):
                        str = f"case 0x{i}{j}{k}{l}: return node_->evaluate(ctx, swl).array.{x}{y}{z}{w}();"
                        print(str, file=file)

def generate_average(file, dim):
    input = ["x", "y", "z", "w"]
    if dim == 1:
        for i, x in enumerate(input):
            str = f"case 0x{i}: return ocarina::vector<float>{{node_->average()[{i}]}};"
            print(str, file=file)
    elif dim == 2:
        for i, x in enumerate(input):
            for j, y in enumerate(input):
                str = f"case 0x{i}{j}: return ocarina::vector<float>{{node_->average()[{i}],node_->average()[{j}]}};"
                print(str, file=file)
    elif dim == 3:
        for i, x in enumerate(input):
            for j, y in enumerate(input):
                for k, z in enumerate(input):
                    str = f"case 0x{i}{j}{k}: return ocarina::vector<float>{{node_->average()[{i}], node_->average()[{j}], node_->average()[{k}]}};"
                    print(str, file=file)
    else:
        for i, x in enumerate(input):
            for j, y in enumerate(input):
                for k, z in enumerate(input):
                    for l, w in enumerate(input):
                        str = f"case 0x{i}{j}{k}{l}: return ocarina::vector<float>{{node_->average()[{i}], node_->average()[{j}], node_->average()[{k}], node_->average()[{l}]}};"
                        print(str, file=file)

if __name__ == "__main__":
    base = dirname(realpath(__file__))
    for dim in range(1, 5):
        with open(f"{base}/slot_swizzle_{dim}.inl.h", "w") as file:
            generate(file, dim)
    
    for dim in range(1, 5):
        with open(f"{base}/slot_average_swizzle_{dim}.inl.h", "w") as file:
            generate_average(file, dim)
            