Setup Code Blocks compiler:
If you get the compiler error: "this_thread is not defined", then you are missing the Mingw with Posix threads.

Windows:
Download mingw-w64 installer
choose w32 and for threads choose posix.
Copy folder path: Default: (C:\Program Files\mingw-w64\x86_64-8.1.0-posix-seh-rt_v6-rev0\mingw64\bin)
Go to code blocks
settings > compiler
select GNU GCC Compiler
Click Copy
Rename to anything ex: "GNU g++ w64 posix compiler"
select the new compiler: "GNU g++ w64 posix compiler"
click toolchain executables
set compiler installtion directory to the parent folder of the bin folder, Defailt: ((C:\Program Files\mingw-w64\x86_64-8.1.0-posix-seh-rt_v6-rev0\mingw64))
Close menu

Project > Build options
select the new compiler: "GNU g++ w64 posix compiler" 

Build and Run should now work.

One set that may be necessary, add the bin to PATH.nh