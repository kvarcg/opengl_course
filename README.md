# Intro Course to OpenGL 3.3

This repo contains source material from the Intro to OpenGL (3.3) lab course, that I taught during my PhD (2011-2015) at the [Deparment of Informatics](https://www.dept.aueb.gr/en/cs) of the [Athens University Economics and Business](https://www.aueb.gr). 

# Requirements
The code requires Windows/Visual Studio and has been tested on VS 2017. The only external libraries used are (older versions) of [FreeGLUT](http://freeglut.sourceforge.net), [GLEW](http://glew.sourceforge.net) and [GLM](https://glm.g-truc.net). These are included in the *3rdparty* folders of each Lab.

# Compiling
Simply open the *.sln* file in the *Solutions* folder and build/run each project. Please note that the relative path used for loading the shaders and the external data (*.obj* files, textures) only works within Visual Studio, so the corresponding folders will need to be moved in order to properly run the code outside the IDE.

# Course Material
Each lab's content is stored in its own folder. Each folder contains fully commented source code (C++/OpenGL/GLSL ) and *.pdf* files directly related to each lab's content, i.e., lab material and exercises to be completed. Any exercise solutions are also included in each lab's folder.

| Lab No | Content |
| --- | --- |
| 1 | Intro to Visual Studio, OpenGL (3.3) and FreeGLUT |
| 2 | Transformations (world, camera, projections)<br/>Intro to GLSL and GLM |
| 3 | Primitive types (triangles, triangle lists, etc.), VAO, VBO.<br/>Depth testing and face culling |
| 4 | Object loader and scene management |
| 5 | Alpha blending, lighting, shading and materials |
| 6 | Texturing (loader), filtering, samplers |
| 7 | FBOs, shadow mapping |
| Extra | Multiple Lights |
