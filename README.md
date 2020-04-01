# IL-2 Graphics Extender (IL2GE)

### Requirements
- OpenGL version 4.5 (https://en.wikipedia.org/wiki/OpenGL#OpenGL_4.5)
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
      StencilBits=1
      ```

### Current bugs/limitations and workarounds
- artifacts with the default water textures in some modpacks  
  **I recommend using Carsmaster's water mod to avoid artifacts (you may need to manually install it, even if it's included in a modpack you are using).**
- dynamic lights cause artifacts - I recommend disabling them in conf.ini:
   ```
   [GLPROVIDER]
   DynamicalLights=0
   ```
- forests are flat (but using a parallax effect) (code for layered forests exists but needs performance improvement)
- misplaced coast lines on some maps
- lighting / atmosphere color could use some tuning, or a better (PBR) algorithm
- no water reflections
- water is flat
- no cloud shadows
- no shore waves / wave foam (code is there but currently disabled) - WIP
- ~~no normal/bump maps~~
- ~~terrain artifacts (e.g. appears lower than it acually is)~~
- ~~objects appear in front of terrain even if they are behind it~~
- ~~forest near/far texture is not blended~~
- ~~distant coast line artifacts on some GPUs~~
- ~~no ground fog on terrain (objects are fogged, so this looks odd)~~
- ~~no cirrus clouds~~
- ~~lighting / atmospheric effect is not applied to objects (planes/buildings/trees/roads/clouds/etc.)~~
- ~~lighting at dusk/dawn/night needs improvement~~
- ~~currently only cloudless weather looks as intended~~

### Installing the latest developement snapshot
Download and run [il2ge-installer.exe](https://gitlab.com/vrresto/il2ge/-/jobs/artifacts/master/raw/il2ge-installer.exe?job=build).
If you get a `404 - Not Found` error, please try again after a few minutes.
