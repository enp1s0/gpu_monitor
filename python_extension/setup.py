from glob import glob
from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension

__version__ = "0.0.1"

ext_modules = [
    Pybind11Extension(
        "gpu_monitor",
        sorted(glob("src/*.cpp")),
    ),
]

setup(
    name="gpu_monitor",
    version=__version__,
    author="enp1s0",
    author_email="mutsuki@momo86.net",
    url="https://gitlab.momo86.net/mutsuki/gpu_logger",
    description="GPU Monitor Python extension",
    long_description="",
    ext_modules=ext_modules,
    zip_safe=False,
)
