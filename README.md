Dependencies
------------

For compiling on Fedora:
```
sudo dnf install wxGTK-devel make gcc-c++
```

For cross-compiling on Fedora for Windows:
```
sudo dnf install make mingw32-gcc-c++ mingw32-wxWidgets3-static mingw32-libpng-static mingw32-libjpeg-turbo-static mingw32-libtiff-static mingw32-zlib-static
```


Building
--------

You may want to fine tune the files Makedefs (if compiling for Linux) or Makedefs.mingw32 (if cross-compiling for Windows).

When cross-compiling for windows, if you are having dumb "Mismatch between the program and library build versions" due to g++ ABI version, see the note in Makedefs.mingw32 for a possible workaround (blame the wxWidgets developers for having such a bad incompatibility detection system, but also your distribution mantainers for not disabling that or caring to recompile their packages).


On Linux, to compile for Linux:
```
make -j8
```

On Linux, to cross-compile for Windows:
```
make -f Makefile.mingw32 -j8
```


Running
-------

Execute the produced executable:
- On Linux: `sueca`
- On Windows: `sueca.exe`
