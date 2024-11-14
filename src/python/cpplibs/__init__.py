from . import ocapi
from . import vsapi
import os
import numpy as np

package_path = os.path.dirname(__file__)

def init_context(backend):
    ocapi.init_context(backend, package_path)