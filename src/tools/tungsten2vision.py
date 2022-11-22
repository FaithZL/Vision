# -*- coding:utf-8 -*-

import json
from pathlib import Path
from posixpath import abspath
from sys import argv
import os
import glm

g_textures = []
g_lights = []

table = {
    "gamma" : 0,
    "filmic" : 1,
    "reinhard" : 2,
    "linear" : 3,
}

def convert_roughness(r):
    return glm.sqrt(r)



if __name__ == "__main__":
    main()