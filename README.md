# IL-2 Graphics Extender (IL2GE)

### Requirements
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

### Installing the latest developement shnapshot
Download and run [il2ge-installer.exe](https://gitlab.com/vrresto/il2ge/-/jobs/artifacts/master/raw/il2ge-installer.exe?job=build)

### Current bugs/limitations and workarounds
- artifacts with the default water textures in some modpacks  
  **I recommend using Carsmaster's water mod to avoid artifacts (you might need to manually install it, even if it is part of a modpack you are using).**
- forests are flat (but using a parallax effect) (code for layered forests exists but needs performance improvement)
- misplaced coast lines on some maps
- no normal/bump maps
- lighting / atmosphere color could use some tuning, or a better (PBR) algorithm
- no water reflections
- water is flat
- no cloud shadows
- no shore waves / wave foam (code is there but currently disabled) - WIP
- ~~terrain artifacts (e.g. appears lower than it acually is)~~
- ~~objects appear in front of terrain even if they are behind it~~
- ~~forest near/far texture is not blended~~
- ~~distant coast line artifacts on some GPUs~~
- ~~no ground fog on terrain (objects are fogged, so this looks odd)~~
- ~~no cirrus clouds~~
- ~~lighting / atmospheric effect is not applied to objects (planes/buildings/trees/roads/clouds/etc.)~~
- ~~lighting at dusk/dawn/night needs improvement~~
- ~~currently only cloudless weather looks as intended~~
