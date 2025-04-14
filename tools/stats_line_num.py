# -*- coding:utf-8 -*-

import os

count = 0

num_file = 0

for root,dirs,files in os.walk(os.path.join(os.getcwd(), "src")):
    for file in files:
        fn = os.path.join(root,file)
        if "ext\\" in fn:
            continue
        if "tests" in fn:
            continue
        if "srgb2spec.h" in fn:
            continue
        if "cpplibs\\" in fn:
            continue
        if "__pycache__\\" in fn:
            continue
        if ".json" in fn:
            continue
        if "metal_ior.inl.h" in fn:
            continue
        if "ltc_sheen_table.h" in fn:
            continue
        if "precomputed_table.h" in fn:
            continue
        if "stats_line_num.py" in fn:
            continue
        if "test.py" in fn:
            continue
        if "_embed.h" in fn:
            continue
        if "jitify" in fn:
            continue
        if ".natvis" in fn:
            continue
        if "json.hpp" in fn:
            continue
        if "ocarina" in fn:
            continue
        try:
            f = open(fn, "r")

            num = len(f.readlines())
            # print(file , num)
            
            count += num
        except :
            print(fn)

        num_file += 1

print("renderer stats ", count, num_file)

r_count = count
r_num_file = num_file

count = 0

num_file = 0

for root,dirs,files in os.walk(os.path.join(os.getcwd(), "src/ocarina/src")):
    for file in files:
        fn = os.path.join(root,file)
        if "ext\\" in fn:
            continue
        if "tests" in fn:
            continue
        if "stats_line_num.py" in fn:
            continue
        if "builtin\\optix\\" in fn:
            continue
        if "jitify" in fn:
            continue
        if "sdk_pt" in fn:
            continue
        if ".natvis" in fn:
            continue
        try:
            f = open(fn, "r")
            count += len(f.readlines())
        except :
            print(fn)

        num_file += 1


print("frame work stats " , count, num_file)

py_count = 0
py_file_num = 0

for root,dirs,files in os.walk(os.path.join(os.getcwd(), "python")):
    for file in files:
        fn = os.path.join(root,file)
        if "toml" in fn:
            continue
        if "cpplibs\\" in fn:
            continue
        if "__pycache__\\" in fn:
            continue
        try:
            f = open(fn, "r")
            py_count += len(f.readlines())
        except :
            print(fn)

        py_file_num += 1


print("python stats " , py_count, py_file_num)


print("total stats: ", count + r_count + py_count, num_file + r_num_file + py_file_num)