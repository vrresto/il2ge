# IL-2 Graphics Extender (IL2GE)

## Requirements
- IL-2 Selector (at least Version 3.4.2)

## Required settings
- OpenGL mode - DirecX is not supported
- conf.ini
[GLPROVIDER]
GL=Opengl32.dll
[Render_OpenGL]
HardwareShaders=1
Water=0

## Current Bugs / Limitations
- terrain artifacts (e.g. appears lower than it acually is)
- objects appear in front of terrain even if they are behind it
- no normal/bump maps
- lighting / atmosphere color could use some tuning, or a better (PBR) algorithm
- no water reflections
- water is flat
- no cloud shadows
- no cirrus clouds
- lighting/atmospheric effect is not applied to objects (planes/buildings/trees/roads/clouds)

I recommend using Carsmaster's water mod.
