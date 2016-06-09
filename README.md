## OpenVNA

This is the header-file for the Akela OpenVNA API, as well as the windows 
DLL files required to interface with a local Akela VNA.

Note that header files are associated with a particular DLL version, so
the header file is in the subdirectory containing the DLL folders.

The correct DLL for your platform (32/64 bit) is also required for the various
demonstration applications that work with the VNA, both the Python and C++ 
versions.

The DLL requires the Microsoft Visual C++ 2013 redistributable package to 
operate properly. This is provided in this repository (see `vcredist_x86.exe` or
`vcredist_x64.exe` for 32 and 64 bit architectures respectively, in the 
`dependencies` folder).
Alternatively, you can download the Visual C++ 2013 from microsoft directly 
here: https://www.microsoft.com/en-us/download/details.aspx?id=40784

----

Our build-process can easily produce linux shared libraries, please let us know 
what distro you use so we can validate compatibility. Drop us a line (or open
a bug report), and we should be able to provide a *.so for your distro of choice
within a few days.
