# COMP3015 – Computer Graphics  
## Coursework 1 – Lighting & Shading (Lab 3)

### 👨‍🎓 Student Information
- Name: RIYA SHAH
- Student ID: 10816845
- Module: COMP3015
- Coursework: CW1 – Lighting & Shading

---

## 📌 Project Overview

This coursework implements advanced lighting techniques in OpenGL using GLSL shaders.  
The project demonstrates the *Phong Reflection Model (ADS)* with spotlight effects and fog applied to a 3D mesh.

The rendered object is a triangulated pig mesh (pig_triangulated.obj).

---

## 🎯 Implemented Features

### ✅ 1. Phong Reflection Model (ADS)
The lighting model includes:

- *Ambient lighting*
- *Diffuse lighting*
- *Specular lighting*

Implemented per-fragment in the fragment shader.

Formula used:

Ambient  = La × Ka  
Diffuse  = Ld × Kd × max(dot(N, L), 0)  
Specular = Ls × Ks × pow(max(dot(R, V), 0), Shininess)

---

### ✅ 2. Spotlight Implementation

A spotlight is implemented using:

- Light position
- Light direction
- Cutoff angle

Fragments outside the spotlight cone receive only ambient lighting.

---

### ✅ 3. Per-Fragment Shading

Lighting calculations are performed in the *fragment shader*, producing smooth highlights and realistic shading.

---

### ✅ 4. Fog Effect (Exponential)

Distance-based exponential fog is implemented:

FogFactor = exp(-density² × distance²)

The final color is blended using GLSL mix() between object color and fog color.

---

## 🗂 Project Structure

COMP3015 Lab 1/
│
├── main.cpp
├── scenebasic_uniform.h
├── scenebasic_uniform.cpp
│
├── shaders/
│   ├── basic_uniform.vert
│   └── basic_uniform.frag
│
├── media/
│   └── pig_triangulated.obj
│
├── src/
│   └── glad.c
│
└── README.md

## 🛠 Technologies Used

- OpenGL 4.6
- GLSL
- GLM
- GLFW
- GLAD
- C++

---

## ▶️ How to Run

1. Open the solution in Visual Studio.
2. Ensure shaders folder is correctly located.
3. Build → Rebuild Solution.
4. Press F5 to run.

---

## 📷 Expected Output

The application renders:

- A blue shaded pig mesh
- Spotlight illumination
- Smooth specular highlights
- Distance-based fog effect

---

## 📚 Learning Outcomes

This coursework demonstrates understanding of:

- Vector lighting calculations
- Surface normals
- Reflection vectors
- Material properties
- Shader programming
- OpenGL rendering pipeline

---

## 🔗 GitHub Repository

Repository link:
https://github.com/riyasah0963/Comp3015-lab1

---