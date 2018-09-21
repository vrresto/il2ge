# IL-2 Graphics Extender (IL2GE)

I recommend using Carsmaster's water mod.

## Requirements
- IL-2 Selector (at least Version 3.4.2)
- Settings:
   - OpenGL mode - DirectX is not supported
   - perfect mode
   - conf.ini:
      ```
      [GLPROVIDER]
      GL=Opengl32.dll
      ```
      ```
      [Render_OpenGL]  
      HardwareShaders=1  
      Water=0
      ```

## Installation
Extract the contents of `install` in the zip file into your IL-2 folder.

## Current Bugs / Limitations
- ~~terrain artifacts (e.g. appears lower than it acually is)~~
- ~~objects appear in front of terrain even if they are behind it~~
- forests are flat (but using a parallax effect) (code for layered forests exists but needs performance improvement)
- no ground fog on terrain (objects are fogged, so this looks odd)
- currently only cloudless weather looks as intended
- forest near/far texture is not blended
- no normal/bump maps
- lighting at dusk/dawn/night needs improvement
- lighting / atmosphere color could use some tuning, or a better (PBR) algorithm
- no water reflections
- water is flat
- no cloud shadows
- no cirrus clouds
- no shore waves / wave foam (code is there but currently disabled) - WIP
- lighting / atmospheric effect is not applied to objects (planes/buildings/trees/roads/clouds/etc.) - WIP
