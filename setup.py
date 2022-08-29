"""
B-Plus Tree Functionality as a C extension
"""
import setuptools

with open("README.md", "r") as f:
    LONG_DESCRIPTION = f.read()

setuptools.setup(
    name="five-one-one-bplus",
    version="0.0.2",
    author="ecowley",
    author_email="erik@stromsy.com",
    description="A module of python containers, implemented in C on top of a B-Plus Tree",
    long_description=LONG_DESCRIPTION,
    long_description_content_type="text/markdown",
    url="https://datascience.stromsy.com",
    packages=[
        "five_one_one_bplus",
    ],
    package_dir={
        "five_one_one_bplus": "python",
    },
    ext_modules=[
        setuptools.Extension(
            "five_one_one_bplus.c",
            [
                "c/general.c",
                "c/array32.c",
                "c/bplusnode.c",
                "c/bplustree.c",
            ],
        ),
    ],
    python_requires=">=3.10",
    classifiers=["Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
    ]
)
