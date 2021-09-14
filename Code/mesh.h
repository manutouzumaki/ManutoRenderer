#ifndef MESH_H
#define MESH_H

// opaque struct define in directx.h and opengl.h
struct mesh;

struct obj
{
    float *Vertices;
    unsigned int VerticesCout;
    unsigned int *Indices;
    unsigned int IndicesCount;
};
   

#endif
