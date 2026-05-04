# SurfaceLab: A Real-Time Shader Material Showcase

## Module

- COMP3015 Games Graphics Pipelines
- Coursework 2

## Project Overview

SurfaceLab is a compact real-time OpenGL shader showcase built in the required
Lab 1 template. The scene focuses on surface appearance, lighting behaviour,
and shader-driven rendering features rather than gameplay systems.

The project is designed as a material and lighting study. A textured ground
plane receives directional shadows, multiple torus meshes demonstrate distinct
physically based responses, one torus uses a procedural noise-driven wood
material, a small particle fountain adds animated motion, and one dedicated
object uses procedural shell-based hair generation. The camera moves around the
scene so that each feature can be observed under the same lighting setup.

## CW2 Feature Set

### 1. Physically Based Rendering

The main shading model is a Cook-Torrance BRDF implemented in
`shader/basic_uniform.frag` using GGX distribution, Schlick-GGX visibility, and
Fresnel-Schlick reflectance.

### 2. Shadow Mapping

A depth-only pass is rendered from the directional light into a shadow map, and
the main fragment shader evaluates that map with a 3x3 PCF kernel.

### 3. Procedural Noise Material

One torus uses a procedural wood-grain material generated entirely in the
fragment shader. A small value-noise and FBM stack is evaluated in object space
and converted into ring and grain patterns, giving the scene an explicit Week 9
style noise feature distinct from the texture-mapped ground.

### 4. Basic 2D Texture Sampling

A procedural checkerboard texture is generated in code and sampled through a
`sampler2D` on the ground plane and one torus material.

### 5. Procedural Hair Generation

One torus is rendered repeatedly in layered shells displaced along the surface
normal. A procedural strand mask and fragment discard create sparse hair-like
coverage, and alpha blending softens the silhouette.

### 6. Particle System

A small CPU-driven particle fountain is rendered as point sprites with its own
shader pass. Particles are respawned, integrated with gravity, and drawn with
soft additive sprites, giving the project a clear Week 7 particle/animation
feature.

## Controls

- `Esc`: quit
- `Space`: toggle scene animation

## Core Files

- `scenebasic_uniform.cpp`: scene setup, animation, textures, shadow map, and render passes
- `scenebasic_uniform.h`: scene members and helper declarations
- `shader/basic_uniform.vert`: world/object-space transforms and shell displacement
- `shader/basic_uniform.frag`: PBR lighting, procedural noise, texture sampling, shadows, hair mask
- `shader/particle.vert`, `shader/particle.frag`: point-sprite particle fountain pass
- `shader/shadow.vert`: depth-only shadow pass vertex shader
- `shader/shadow.frag`: minimal shadow pass fragment shader

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
