import setuptools

with open("README.md", "r") as fh:
    long_description = fh.read()

setuptools.setup(
    name="megaio",
    version="1.3.1",
    author="Alexandru Burcea",
    author_email="olcit@gmail.com",
    description="A set of functions to control Sequent Microsystems MegaIO board",
	license='MIT',
    url="https://github.com/SequentMicrosystems/megaio-rpi",
    packages=setuptools.find_packages(),
    classifiers=[
        "Programming Language :: Python :: 2/3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
    ],
)