Seven Kingdoms: Ancient Adversaries - Visual Studio Readme
----------------------------------------------------------
Copyright 2014 Richard Dijk (MicroVirus)


This branch of the 7kaa project runs parallel to the 7kaa master branch
and includes project files for Microsoft Visual C++ 2010.

Most of the work is done, there are just a few more steps to be taken
before you can compile and debug your own version of the source code
with VC++, all pertaining to including the used libraries.

7kaa still has several backends, but only the following libraries/backends
are 'supported' by this branch (though others might work).
- SDL2           Tested using version 2.0.1
- OpenAL32       I *think* tested with version 1.1, but Creative Labs has left OpenAL a mess,
                  so even acquiring the library files is problematic.
- ENet           Tested using version 1.3.10



Setting up the libraries
------------------------

--- SDL2 ----
1. Download the latest version (or the tested version) of SDL (http://libsdl.org/).
2. Select the Visual C++ Development Library and download it.
3. Extract the files to 7kaa\SDL, such that the directory structure looks like 7kaa\SDL\lib, 7kaa\SDL\include.
   The compiler and linker can now find SDL2.
4. To enable running the compiled executable, copy the DLL file 7kaa\SDL\lib\x86\SDL2.dll to 7kaa\data.
   This will allow the executable to find SDL2.dll, as the Debugger has been set up to use the 7kaa\data directory
   as the working directory, allowing you to directly Run from the IDE.

--- OpenAL32 ---
It seems currently nearly impossible to find a development version, or documentation for that matter, of OpenAL.
The include files and library are therefore directly included in the branch, under 7kaa\OpenAL.
To run the executable OpenAL32.dll needs to be present on your system. If 7kaa.exe can not find it, search for a 32-bits
version of OpenAL32.dll on your system (certain games include it) and copy it to 7kaa\data\ (just like SDL2.dll).

--- enet ---
1. Download the latest version (or the tested version) of ENet (http://enet.bespin.org/).
2. Extract enet.lib, and the contents of 'include' to 7kaa\enet (no subdirectories this time; all the extracted files
   go directly into 7kaa\enet).
ENet is a static link library, so no DLL is required.



Your setup should now be working. For any questions, remarks, suggestion or to share your work,
please visit the 7kaa open source project at http://www.7kfans.com/forums/index.php.



Disclaimer: Visual Studio is not directly supported by the 7kaa open source project. Therefore it can not be
guaranteed that future updates will directly work with the Visual Studio project file.
I will try to keep this branch up-to-date as the master branch advances.