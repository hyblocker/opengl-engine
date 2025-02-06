# OpenGL Game Engine

This is a simple OpenGL game engine for the University class [CPS3231, Graphics Programming](https://www.um.edu.mt/courses/studyunit/CPS3231), delivered by the University of Malta.

## Engine features

- Mesh loading.

- Graphics API agnostic abstraction (`gpu::IDevice`).

- Basic forward rendering.

- Early-Z discard for skybox rendering.

- Resource caching.

- Multiple lights rendering.

- An infinite far plane perspective camera.

- Normalised inverse-Z depth buffer for improved depth precision.

- Box2D physics.

- Particles.

- Simple UI rendering.

- OpenGL debug utilities. Draw calls and resource allocation are annotated so that debug names appear in graphics debuggers such as NVIDIA NSight and RenderDoc.

- Layer system capable of blocking inputs.

- Dear ImGUI debug layer. Used to build a runtime representation of the scene and entities / components to inspect their values for correctness.

- Input system.

A number of techniques are primitive and could be better implemented, but weren't due to time constraints.
