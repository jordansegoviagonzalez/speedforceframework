from ._internal import _bindings
from .engine import Engine, Model # Assuming engine.py wraps the C++ classes or exposes them

__version__ = "0.1.0"

# Expose the raw C++ bindings if needed, but usually we prefer the python wrappers
# from ._internal._bindings import Tensor 

__all__ = ["Engine", "Model", "__version__"]
