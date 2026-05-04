# SurfaceLab: A Real-Time Shader Material Showcase

## Module

- COMP3015 Games Graphics Pipelines
- Coursework 2

## Project Overview

SurfaceLab is a compact real-time OpenGL shader showcase built in the required
Lab 1 template. The project extends the earlier coursework pipeline into a small
scene that focuses on surface appearance, lighting behaviour, and advanced
shader-driven rendering features rather than gameplay systems.

The scene is designed as a material and lighting study. A textured ground plane
receives directional shadows, multiple torus meshes demonstrate different
physically based material responses, and one dedicated object uses procedural
shell-based hair generation to show a more experimental shader technique. The
camera moves around the scene so that each feature can be observed under the
same lighting setup.

## Implemented Features

### 1. Physically Based Rendering

The main shading model is a Cook-Torrance physically based BRDF implemented in
the fragment shader. The project uses:

- GGX normal distribution
- Schlick-GGX geometry visibility
- Fresnel-Schlick reflectance
- material parameters for albedo, metallic, roughness, and ambient occlusion

This allows the torus models to react differently to the same lighting setup,
making the surface study clear and easy to evaluate.

### 2. Shadow Mapping

A depth-only shadow pass is rendered from the directional light into a shadow
map. The main lighting shader transforms each fragment into light space,
compares fragment depth against the stored depth map, and attenuates direct
lighting when the fragment is occluded. A small PCF kernel is used to soften
the result and reduce aliasing.

### 3. Basic 2D Texture Sampling

A procedural checkerboard texture is generated in code and uploaded as a 2D
texture. The textured ground plane and one of the torus materials sample this
texture through a `sampler2D` in the fragment shader. This demonstrates
successful texture setup, binding, coordinate interpolation, and fragment-stage
sampling.

### 4. Procedural Hair Generation

The project includes a procedural shell-based hair effect on a dedicated torus.
The object is rendered repeatedly in layered shells displaced along the surface
normal. In the fragment shader, a procedural strand mask is produced from hashed
texture-space cells and fragment discard is used to create sparse strand-like
coverage. Alpha blending is applied across shell layers to produce a soft,
hair-like silhouette.

## Scene Design

The scene is intentionally simple so the shader features are easy to read:

- a checkerboard ground plane improves scale perception and makes shadows easy
  to judge
- several torus objects present distinct PBR material combinations
- one torus demonstrates procedural shell hair
- a directional light provides stable shadows
- animated point lights add local highlights and color variation
- the orbiting camera reveals material changes from different viewing angles

This approach keeps attention on shading quality and avoids clutter that would
make the graphics features harder to assess.

## Technical Structure

### Core Source Files

- `scenebasic_uniform.cpp`
  scene setup, animation, light setup, texture generation, shadow-map setup,
  and render passes
- `scenebasic_uniform.h`
  scene class members and helper function declarations
- `shader/basic_uniform.vert`
  vertex processing, world-space transforms, shell displacement, light-space
  transform preparation
- `shader/basic_uniform.frag`
  PBR lighting, texture sampling, shadow evaluation, and procedural hair mask
- `shader/shadow.vert`
  depth-only vertex stage for the shadow pass
- `shader/shadow.frag`
  minimal shadow-map fragment stage

### Helper Geometry

- `helper/trianglemesh.*`
  indexed drawable mesh base class
- `helper/plane.*`
  ground plane mesh
- `helper/torus.*`
  torus mesh generation used for the material studies

## Rendering Pipeline Summary

1. A checkerboard texture is generated and uploaded.
2. A depth texture and framebuffer are created for shadow mapping.
3. Each frame, the scene is rendered once from the light’s point of view into
   the shadow map.
4. The main pass renders the scene from the camera view.
5. The main fragment shader combines:
   PBR lighting
   directional shadows
   texture sampling
   procedural hair shell masking
6. Tone mapping and gamma correction are applied before final output.

## Build Instructions

1. Open `Lab 1/Project_Template.sln`.
2. Select `x64`.
3. Build either `Debug` or `Release`.
4. Run from the `Lab 1` working directory so shader paths resolve correctly.

The project is configured to use:

- `C:\Users\Public\OpenGL\include`
- `C:\Users\Public\OpenGL\lib`
- `C:\Users\Public\OpenGL\src`

## Verification

Verified locally:

- `Debug|x64` build: success
- `Release|x64` build: success
- runtime smoke test: success

## Project Identity

This coursework should be presented as:

`SurfaceLab: A Real-Time Shader Material Showcase`

It is best described as a technically focused shader demonstration that studies
how different rendering techniques affect surface appearance under shared scene
conditions.
