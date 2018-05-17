# agsjoy
Joystick for Adventure Game Studio Engine in Linux. *libagsjoy written by qptain_Nemo, for Linux, compatible with Wyz agsjoy*.

I saw this plugin used in Linux version of Until I Have You, Dualnames told me to ask qptain Nemo for source. He sent me the source and told me I could share and stamp a MIT license on it, so here it is.

This code  uses SDL2 and it makes your game depend on SDL2.

Install dependencies sdl2, g++, libc6-dev . 

**Attention** right now, the **Makefile is not working yet**. Use the `build.sh` script in the folder. Tested only in Ubuntu 16.04.

If you want to make in Ubuntu 16.04:

    sudo apt install libsdl2-dev g++ libc6-dev libc6-dev-i386 git make wget
    git clone git@github.com:ericoporto/agsjoy.git
    cd agsjoy
    wget https://raw.githubusercontent.com/adventuregamestudio/ags/v.3.4.1.12/Engine/plugin/agsplugin.h
    make
    

It should work with regular ags linux port.

**about 32-bit linux** For now, the solution is to use a 32 bit Ubuntu VM until I can findout how to solve the dependencies for multiarch in Ubuntu 16.04

If you want to build for 32-bit systems and you are on amd64 install, additionally, install libc6-dev-i386, lib32stdc++-5-dev.


[**Go here to grab the latest release**](https://github.com/ericoporto/agsjoy/releases)!

## how to build agsjoy for Windows

Extract `Developent.zip` in `C:/` - you should end up with a `C:/Development/` directory. This will give you the dependencies for SDL2.

Install Visual Studio 2017 Community, install the C++ stuff.

Open the `agsjoy.sln` here in Visual Studio 2017. Press build Release, select win32 when building the .dll since the shipped SDL2 is x86. 
