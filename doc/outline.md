Raymarching the Hierarchical A-Buffer
===============================================================

* [ ] Abstract

Introduction
------------

* [ ] Rendering paradigms (raytracing and forward rendering)
* [ ] Screen-space raytracing (definition, applications, limitations)

Method overwiew
---------------

* [ ] OpenGL rendering pipeline
* [ ] Z-buffer and A-Buffer
* [ ] Hierarchical depth buffer
* [ ] Constant step raymarching
* [ ] Raymarching in a hierarhical depth buffer
* [ ] Cone tracing for rough reflections _(maybe)_
* [ ] Comparison with voxelization-based algorithms

Implementation
--------------

* [ ] Shader execution model
* [ ] A-buffer implementation on the GPU using linked lists
* [ ] Initial rasterization stage (building the first layer)
* [ ] Building hierarchy levels
* [ ] Coordinate transformations
* [ ] Constant step raymarching in a single-layer depth buffer
* [ ] Raymarching in a hierarchical single-layer depth buffer
* [ ] Raymarching in a hiararchical min-max depth buffer
* [ ] Raymarching in a hierarchical A-buffer
* [ ] Cone tracing _(maybe)_
* [ ] Performance overhead reduction with frame latency

Evaluation
----------

* [ ] Full screen performance
* [ ] Iteration count
* [ ] Applications to reflections
* [ ] Applications to ambient occlusion, indirect li

