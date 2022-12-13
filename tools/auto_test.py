# -*- coding:utf-8 -*-

import os
import subprocess

release_dir = "cmake-build-release"

def get_exe_path():
    return os.path.join(os.getcwd(), release_dir, "bin", "vision-gui.exe")


def run_scene(scene_file):
    exe = get_exe_path()
    # os.chdir(os.path.dirname(exe))
    # print(os.getcwd())
    print(scene_file)
    # os.system(exe + " -s E:/work/compile/Vision/res/cbox-sss.json ")
    
    # ret = subprocess.Popen([exe , "-h"])
    # ret.wait()
    # print(ret.returncode)

def main():
    for root,dirs,files in os.walk(os.path.join(os.getcwd(), "res/test_case")):
        for file in files:
            if file.endswith(".json"):
                run_scene(os.path.realpath(file))
                return
    
if __name__ == "__main__":
    main()