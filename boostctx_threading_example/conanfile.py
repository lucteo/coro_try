from conans import ConanFile, CMake, tools
import re
from os import path


class CppAsyncRecipe(ConanFile):
    name = "boostctx_threading_example"
    description = "Playing around with Boost.Context"
    author = "Lucian Radu Teodorescu"
    topics = ()
    homepage = "https://github.com/lucteo/coro_try"
    url = "https://github.com/lucteo/coro_try"
    license = "MIT License"
    settings = "compiler", "cppstd"  # Header only - compiler only used for flags
    tool_requires = ["boost/1.79.0", "tracy-interface/0.1.0", "context_core_api/1.0.0"]
    # exports_sources = "include/*"
    generators = "cmake"

    # def validate(self):
    #     tools.check_min_cppstd(self,"20")

    def set_version(self):
        self.version = f"0.1.0"

    def package(self):
        self.copy("*.hpp")

    def package_info(self):
        # Make sure to add the correct flags for gcc
        if self.settings.compiler == "gcc":
            self.cpp_info.cxxflags = ["-fcoroutines", "-Wno-non-template-friend"]


# from <root>/.build/ directory, run:
#   > conan install .. --build=missing -s cppstd=20