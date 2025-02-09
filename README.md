# CrashInfo
An .asi plugin for (mainly GoldSrc) games which reports useful (at least for developers) info about a game crash. 

# HOW TO BUILD
Just copy the repository onto your drive, open up `StarLoader_CrashInfo\CrashInfo.sln` file with MSVS 2015/2017/2019/2022 and hit Ctrl+Shift+B.

The artifact will be in `StarLoader_CrashInfo\Debug` or `StarLoader_CrashInfo\Release` folder.

Rename `CrashInfo.dll` into `CrashInfo.asi` and follow `HOW TO INSTALL`

# HOW TO INSTALL
Either follow `HOW TO BUILD` to build an artifact, or download one from Releases.

This plugin requires any .asi loader in order to be loaded, if you don't have any you can use mine -- https://github.com/autisoid/StarLoader

Drop the `CrashInfo.asi` file into `starloader` folder located in your game ROOT folder (e.g. `C:\Program Files (x86)\Steam\steamapps\common\Sven Co-op\starloader`) and launch the game.

In case of a crash, a message box with info will appear.