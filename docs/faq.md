# FAQ

> [!CAUTION]
> Please read this before using this project, or asking for any support!

## Do I need to know programming to use Pyrite64?
Yes, C++ / C knowledge is required.<br>
Generic visual programming languages are not implemented.<br>
The node-graph shown in the repo / video is *only* for scripting sequences of events. 

## Can I make 2D games?
No.<br>
Any kind of 2D support is still WIP and is currently done in code.

## Can I export to PC?
No. The only target is N64.

## Can I mod existing N64 games?
No. This project is only for making new homebrew.

## Can I emulate games in Project64 / [any android emulator]? 
No. This requires accurate emulation.<br>
Recommended emulators are: [Ares (v147 or newer)](https://ares-emu.net/) and [gopher64](https://github.com/gopher64/gopher64). 

## Can I import any 3D models from blender?
Almost, materials need to be made with fast64 however.<br> 

Use blender 4.0+ (not 5.0!), and install: [fast64](https://github.com/Fast-64/fast64).<br>
For setting during export check this note here:
https://github.com/HailToDodongo/tiny3d?tab=readme-ov-file#gltf-model-import
Make sure textures are .png files present in the `assets` folder of your project.<br>
The tiny3d repo also contains example .blend files.

## Can i sell games made with it / will i get sued?
I'm not a lawyer, and the following is not legal advice.<br>
Pyrite64 itself is licensed under MIT, and i put no restrictions on what you can do with games.<br>
The SDKs used, namely libdragon and tiny3d have similar licenses.<br>
Pyrite64 does **NOT** use any proprietary SDKs, like some projects you may have heard of in the past.<br>
So in general is should be safe to sell games if you whish.