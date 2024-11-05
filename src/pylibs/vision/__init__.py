from . import ocapi
import os

package_path = os.path.dirname(__file__)


def create_device(name : str):
    return ocapi.Device.create(name, package_path)