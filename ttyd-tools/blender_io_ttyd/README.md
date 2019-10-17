# Blender exporter for Paper Mario: The Thousand Year Door 

This tool and this documentation is a work-in-progress.

If after reading this document you have unanswered questions, there is a dedicated [Paper Mario TTYD Modding Discord server](https://discord.gg/geVf9UK) that you can join.

## Installation
This addon was tested with Blender 2.80.
Copy the folder `io_scene_ttyd` in this directory into the `scripts/addons/` directory of your Blender installation, then enable it under *Edit*, *Preferences...*, *Add-ons*.

## Playtesting
To actually playtest your map in-game, you can use a tool like GCRebuilder to place the exported map files (`d`, `c`, `t`) in a subfolder of the disc image's `m` folder.
You can either replace an existing map which you can then just go to in the game, or you can put it in as a completely new map. If you elect to go with the second option, you can use e.g. a REL GCI mod to load the map up dynamically, such as the one in the `rel` folder of this repository.

## Usage
Different Blender collections are used to map objects to different functions in the map and must be selected accordingly when exporting. All collections must be present even if unused. Objects should not be in more than one of these functional collections.

### Map and Hit
The Map and Hit collections contain render and collision geometry contained in the main data file (`d`). The parent-child hierarchy will be reflected in the exported joint graph, so objects contained should either not be parented or be parented to another object in the same collection. 
I recommend you use linked objects (Alt-D by default) to reduce the amount of duplicated objects between Map and Hit collections.

#### Materials
Blender material node trees will be converted to TTYD materials on a best-effort basis. If a Blender material does not map to a TTYD material, it will not be converted. In order to map to a valid TTYD material, one of the following setups must be used.

##### Shader setup
The shader output must be connected to either
* a Diffuse Shader if there is no transparency [Example](https://i.imgur.com/Z4h8Nsi.png)
* a Mix Shader of a Transparent Shader and a Diffuse Shader if there is transparency [Example](https://i.imgur.com/AB4ON5f.png)
The Color input of the Diffuse Shader node is the color source, with the Fac input of the Mix Shader being the alpha source if present.
If the material is opaque, then the color input may be left unconnected and set to a constant color, terminating the material. [Example](https://i.imgur.com/aIR6laH.png)

##### Vertex color setups
If vertex colors are to be used, the color input may be connected to
* an Attribute node mapping to a vertex color. In this case, the node tree ends here and the material must be opaque. [Example](https://i.imgur.com/Gq6OI95.png)
* a MixRGB node set to Multiply with it's Fac input set to 1.0, with one of the Color sockets connected to an Attribute node mapping to a vertex color. In this case, the Color socket that is not connected to an Attribute node becomes the new color input. [Example](https://i.imgur.com/gOnshfr.png)
Note that at this time, no alpha channel is supported on vertex colors due to a limitation in Blender 2.80. 

##### TEV setups
The color and alpha inputs must be connected to
 * a single sampler setup.
In future, more TEV modes with more complicated multi-material blending functions will be supported here.

##### Samplers
Every sampler setup in the above TEV setups must end in an Image Texture node. The Image Texture node's Vector input may optionally be connected to a Mapping Node specifying a texture coordinate transform for that sampler. The Vector input of that mapping node may optionally be connected to a UV Map node specifying which UV map to use for that sampler. The Image Texture node's Vector input may also be directly connected to a UV Map node without a Mapping node. [Example](https://i.imgur.com/jHmvPse.png)

##### Blend mode
The Blend Mode set in the *Settings* panel of the Eevee material settings maps to the TTYD blend mode. Opaque, Alpha-Clip and Alpha-Blend are supported. If the material has transparency, the Blend Mode must be set to either Alpha Blend or Alpha Clip.

#### Animations
Map object transform and texture coordinate transform animations are currently supported. To export an animation, push all the relevant Blender animation data into Actions and put them into NLA Tracks. All NLA tracks of the same name will be packed into one TTYD animation.

### Camera
The Camera collection contains the camera marker meshes and curves that will end up in the camera road file (`c`).
A camera curve defines the path the camera should follow in order to track Mario and map to Blender curve objects. There is exactly one curve active at any one time. A curve is considered to be active when any of the markers attached to it is the one Mario is standing in.
Markers are meshes that define "zones" of which curves should be active depending on where Mario stands. The first marker mesh that is hit when projecting Mario's position downwards is considered active. This usually just means that you should put the marker slightly below where Mario should be standing in order to activate it.
Every curve must have at least one marker. To define the markers for a curve, add custom properties on the Blender object (the object data-block, **not** the Curve data-block) named "marker0", "marker1", etc. with the names of the marker objects for that curve. Only "marker0" is required.
TODO: Other curve properties

## Known limitations
* No support for:
 * Lights
 * Fog
 * Different TEV modes
 * Hit attributes
 * Different texture formats (partially implemented)
 * Material blend alpha (partially implemented)