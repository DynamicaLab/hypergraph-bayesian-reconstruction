try:
    from skbuild import setup
except ImportError:
    import sys
    print(
        "Please update pip, you need pip 10 or greater,\n"
        " or you need to install the PEP 518 requirements in pyproject.toml yourself",
        file=sys.stderr,
    )
    raise

from setuptools import find_packages

setup(
    name='pygrit',
    version='1.0.0',
    author='Simon Lizotte',
    description='Python GRaph Inference of Triangles: inference framework to reconstruct the hypergraph structure of noisy observations.',
    zip_safe=False,
    packages=["pygrit"],
    cmake_args=["-DBUILD_BINDINGS=ON"],
    include_package_data=True,
    exclude_package_data={'': ["__pycache__"]},
    python_requires=">=3.6",
)
