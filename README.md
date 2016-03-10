## OpenVNA

This is the header-file for the Akela OpenVNA API, as well as the windows 
DLL files required to interface with a local Akela VNA.

Note that header files are associated with a particular DLL version, so
the header file is in the subdirectory containing the DLL folders.

The correct DLL for your platform (32/64 bit) is also required for the various
demonstration applications that work with the VNA, both the Python and C++ 
versions.

----

Our build-process can easily produce linux shared libraries, please let us know 
what distro you use so we can validate compatibility. Drop us a line (or open
a bug report), and we should be able to provide a *.so for your distro of choice
within a few days.
