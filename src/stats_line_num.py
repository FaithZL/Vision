# -*- coding:utf-8 -*-

import os

count = 0

com = 0

inCom = False

num_file = 0

for root,dirs,files in os.walk(os.path.join(os.getcwd(), "src")):
    for file in files:
        fn = os.path.join(root,file)
        if "ext\\" in fn:
            continue
        if "tests" in fn:
            continue
        if "stats_line_num.py" in fn:
            continue
        if "_embed.h" in fn:
            continue
        if "jitify" in fn:
            continue
        if ".natvis" in fn:
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


# print(count, num_file)

import math

Inv2Pi = 1 / (2 * math.pi)
InvPi = 1 / math.pi

def spherical_phi(x, y):
    p = math.atan2(y, x);
    return p + 2 * math.pi if p < 0 else p

def spherical_theta(z):
    return math.acos(z);

u = spherical_phi(-0.086723,0.132887) * Inv2Pi

v = spherical_theta(-0.981263) * InvPi

print(u, v)