from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
import sys
import os
import setuptools

__version__ = '1.0.0'


class get_pybind_include(object):
    """Helper class to determine the pybind11 include path

    The purpose of this class is to postpone importing pybind11
    until it is actually installed, so that the ``get_include()``
    method can be invoked. """

    def __init__(self, user=False):
        self.user = user

    def __str__(self):
        import pybind11
        return pybind11.get_include(self.user)


ext_modules = [
    Extension(
        'pygrit',
        include_dirs=[
            get_pybind_include(),
            get_pybind_include(user=True),
            "include",
            "include/SamplableSet/src",
            "python_wrapper"
        ],
        sources=[
            "python_wrapper/additional_utility.cpp",
            "python_wrapper/hypergraph_generation.cpp",
            "python_wrapper/metrics.cpp",
            "python_wrapper/models.cpp",
            "python_wrapper/observations_generation.cpp",
            "python_wrapper/pybind_main.cpp",

            "src/utility.cpp",
            "src/trianglelist.cpp",
            "src/hypergraph.cpp",
            "src/gibbs_base.cpp",
            "src/generator.cpp",

            "src/inference-models/phg.cpp",
            "src/inference-models/pes.cpp",
            "src/inference-models/per.cpp",

            "src/observations-models/poisson_hypergraph.cpp",
            "src/observations-models/poisson_edgestrength.cpp",

            "src/hypergraph-models/independent_hyperedges.cpp",
            "src/hypergraph-models/edgestrength.cpp",
            "src/hypergraph-models/gilbert.cpp",

            "src/parameters-samplers/poisson_edgestrength.cpp",
            "src/parameters-samplers/poisson_gilbert.cpp",
            "src/parameters-samplers/poisson_independent_hyperedges.cpp",

            "src/proposers/twosteps_edges.cpp",
            "src/proposers/sixsteps_hypergraph.cpp",

            "src/proposers/edge-choosers/uniform_edge_chooser.cpp",
            "src/proposers/edge-choosers/weighted_two-layers_chooser.cpp",
            "src/proposers/edge-choosers/weighted_unique_chooser.cpp",

            "src/proposers/triangle-choosers/observations_by_pair_chooser.cpp",
            "src/proposers/triangle-choosers/uniform_triangle_chooser.cpp",
        ],
        language='c++',
        extra_objects=["./include/SamplableSet/src/build/libsamplableset.a"]
    ),
]


# As of Python 3.6, C Compiler has a `has_flag` method.
# cf http://bugs.python.org/issue26689
def has_flag(compiler, flagname):
    """Return a boolean indicating whether a flag name is supported on
    the specified compiler.
    """
    import tempfile
    with tempfile.NamedTemporaryFile('w', suffix='.cpp') as f:
        f.write('int main (int argc, char **argv) { return 0; }')
        try:
            compiler.compile([f.name], extra_postargs=[flagname])
        except setuptools.distutils.errors.CompileError:
            return False
    return True


def cpp_flag(compiler):
    """Return the -std=c++[11/14/17] compiler flag.

    The newer version is prefered over c++11 (when it is available).
    """
    flags = ['-std=c++17', '-std=c++11']

    for flag in flags:
        if has_flag(compiler, flag):
            return flag

    raise RuntimeError('Unsupported compiler -- at least C++11 support '
                       'is needed!')


class BuildExt(build_ext):
    """A custom build extension for adding compiler-specific options."""
    c_opts = {
        'msvc': ['/EHsc'],
        'unix': [],
    }
    l_opts = {
        'msvc': [],
        'unix': [],
    }

    if sys.platform == 'darwin':
        darwin_opts = ['-stdlib=libc++', '-mmacosx-version-min=10.7']
        c_opts['unix'] += darwin_opts
        l_opts['unix'] += darwin_opts

    def build_extensions(self):
        ct = self.compiler.compiler_type
        opts = self.c_opts.get(ct, [])
        link_opts = self.l_opts.get(ct, [])
        if ct == 'unix':
            opts.append('-DVERSION_INFO="%s"' % self.distribution.get_version())
            opts.append(cpp_flag(self.compiler))
            if has_flag(self.compiler, '-fvisibility=hidden'):
                opts.append('-fvisibility=hidden')
        elif ct == 'msvc':
            opts.append('/DVERSION_INFO=\\"%s\\"' % self.distribution.get_version())
        for ext in self.extensions:
            ext.extra_compile_args = opts
            ext.extra_link_args = link_opts
        build_ext.build_extensions(self)


setup(
    name='pygrit',
    version=__version__,
    author='Simon Lizotte',
    ext_modules=ext_modules,
    install_requires=['pybind11>=2.3'],
    setup_requires=['pybind11>=2.3'],
    cmdclass={'build_ext': BuildExt},
    zip_safe=False,
)
