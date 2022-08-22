"""
B-Plus Tree Functionality as a C extension
"""
import setuptools
from Cython.Build import cythonize

with open("README.md", "r") as f:
    LONG_DESCRIPTION = f.read()

setuptools.setup(
    name="five-one-one-bplus",
    version="0.0.1",
    author="ecowley",
    author_email="erik@stromsy.com",
    description="a collection of helper methods",
    long_description=LONG_DESCRIPTION,
    long_description_content_type="text/markdown",
    url="https://datascience.stromsy.com",
    packages=[
        "five_one_one_bplus",
    ],
    package_dir={
        "five_one_one_bplus": "source",
    },
    ext_modules=[
        setuptools.Extension(
            "five_one_one_bplus.c",
            ["c/bplus.c",],
        ),
    ],
    python_requires=">=3.8",
    classifiers=["Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
    ]
)
