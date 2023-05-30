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
        if "srgb2spec.h" in fn:
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


print(count, num_file)

# import _thread
# import time

# # 为线程定义一个函数
# def print_time( threadName, delay):
#     count = 0
#     while count < 5:
#         time.sleep(delay)
#         count += 1
#         print ("%s: %s" % ( threadName, time.ctime(time.time()) ))

# # 创建两个线程
# try:
#     _thread.start_new_thread( print_time, ("Thread-1", 2, ) )
#     _thread.start_new_thread( print_time, ("Thread-2", 4, ) )
# except:
#     print ("Error: 无法启动线程")

# while 1:
#     pass