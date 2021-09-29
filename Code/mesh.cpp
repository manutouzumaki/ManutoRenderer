static obj
LoadOBJFile(char *OBJFileName, arena *Arena)
{
    obj Result = {};
    void* OBJFile = ReadEntireFile(OBJFileName, NULL);
    if(!OBJFile)
    {
        OutputDebugString("ERROR::LOADING::OBJ\n");
        return Result;
    }
    
    int VerticesCount = 0; 
    int TextureCoordsCount = 0;
    int NormalsCount = 0;
    int IndexCount = 0;

    // first we loop through the entire file and get the size of the mesh
    char *Line = (char *)OBJFile;
    while(*Line != '\0')
    { 
        if(StringCompare(Line, "v ", 2))
        {
            ++VerticesCount;
        }
        if(StringCompare(Line, "vt ", 3))
        {
            ++TextureCoordsCount; 
        }
        if(StringCompare(Line, "vn ", 3))
        {
            ++NormalsCount;
        }
        if(StringCompare(Line, "f ", 2))
        {
            ++IndexCount;
        }    
        
        int Counter = 0;
        char *Letter = Line;
        while(*Letter != '\n')
        {
            ++Counter;
            ++Letter;
        }
        Line += Counter + 1;
    }
    
    // get the memory
    float *TempVertices = (float *)PlatformAllocMemory((VerticesCount*3)*sizeof(float));
    float *TempTextureCoords = (float *)PlatformAllocMemory((TextureCoordsCount*2)*sizeof(float));
    float *TempNormals = (float *)PlatformAllocMemory((NormalsCount*3)*sizeof(float));
    int *TempVIndex = (int *)PlatformAllocMemory((IndexCount*3)*sizeof(int));
    int *TempTIndex = (int *)PlatformAllocMemory((IndexCount*3)*sizeof(int));
    int *TempNIndex = (int *)PlatformAllocMemory((IndexCount*3)*sizeof(int));
    
    // second loop through the file to ge the values 
    float *VerticesPtr = TempVertices;
    float *TextureCoordsPtr = TempTextureCoords;
    float *NormalsPtr = TempNormals;
    int *VIndexPtr = TempVIndex;
    int *TIndexPtr = TempTIndex;
    int *NIndexPtr = TempNIndex;
    Line = (char *)OBJFile;
    while(*Line != '\0')
    {   
        if(StringCompare(Line, "v ", 2))
        {
            float X, Y, Z;
            sscanf(Line, "v %f %f %f", &X, &Y, &Z);
            *VerticesPtr++ = X;
            *VerticesPtr++ = Y;
            *VerticesPtr++ = Z;
        }
        if(StringCompare(Line, "vt ", 3))
        {
            float U, V;
            sscanf(Line, "vt %f %f", &U, &V);
            *TextureCoordsPtr++ = U;
            *TextureCoordsPtr++ = V;
        }
        if(StringCompare(Line, "vn ", 3))
        {
            float X, Y, Z;
            sscanf(Line, "vn %f %f %f", &X, &Y, &Z);
            *NormalsPtr++ = X;
            *NormalsPtr++ = Y;
            *NormalsPtr++ = Z;
        }
        if(StringCompare(Line, "f ", 2))
        {
            int VIndex[3];
            int TIndex[3];
            int NIndex[3];
            sscanf(Line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                   &VIndex[0], &TIndex[0], &NIndex[0],
                   &VIndex[1], &TIndex[1], &NIndex[1],
                   &VIndex[2], &TIndex[2], &NIndex[2]);
            *VIndexPtr++ = VIndex[0] - 1;
            *VIndexPtr++ = VIndex[1] - 1;
            *VIndexPtr++ = VIndex[2] - 1;
            *TIndexPtr++ = TIndex[0] - 1;
            *TIndexPtr++ = TIndex[1] - 1;
            *TIndexPtr++ = TIndex[2] - 1;
            *NIndexPtr++ = NIndex[0] - 1;
            *NIndexPtr++ = NIndex[1] - 1;
            *NIndexPtr++ = NIndex[2] - 1;
        }  

        int Counter = 0;
        char *Letter = Line;
        while(*Letter != '\n')
        {
            ++Counter;
            ++Letter;
        }
        Line += Counter + 1;
    }

    // this is store in the permanent storage of the program  
    Result.Vertices = (float *)PushArray(Arena, (3*8*IndexCount), float);
    float *VertexBufferPtr = Result.Vertices;
    VIndexPtr = TempVIndex;
    TIndexPtr = TempTIndex;
    NIndexPtr = TempNIndex;
    for(int Index = 0;
        Index < 3*IndexCount;
        ++Index)
    {
        int VIndex = *VIndexPtr * 3;
        *VertexBufferPtr++ = TempVertices[VIndex + 0];
        *VertexBufferPtr++ = TempVertices[VIndex + 1];
        *VertexBufferPtr++ = TempVertices[VIndex + 2];
        ++VIndexPtr;
        int TIndex = *TIndexPtr * 2;
        *VertexBufferPtr++ = TempTextureCoords[TIndex + 0];
        *VertexBufferPtr++ = TempTextureCoords[TIndex + 1];
        ++TIndexPtr;
        int NIndex = *NIndexPtr * 3;
        *VertexBufferPtr++ = TempNormals[NIndex + 0];
        *VertexBufferPtr++ = TempNormals[NIndex + 1];
        *VertexBufferPtr++ = TempNormals[NIndex + 2];
        ++NIndexPtr;
    }
    
    Result.VerticesCount = 3*8*IndexCount;

    PlatformFreeMemory(TempVertices);
    PlatformFreeMemory(TempTextureCoords);
    PlatformFreeMemory(TempNormals);
    PlatformFreeMemory(TempVIndex);
    PlatformFreeMemory(TempTIndex);
    PlatformFreeMemory(TempNIndex);
    PlatformFreeMemory(OBJFile);

    return Result;
}


static obj
LoadOBJData(void *OBJFile, arena *Arena)
{
    obj Result = {};
    
    int VerticesCount = 0; 
    int TextureCoordsCount = 0;
    int NormalsCount = 0;
    int IndexCount = 0;

    // first we loop through the entire file and get the size of the mesh
    char *Line = (char *)OBJFile;
    while(*Line != '\0')
    { 
        if(StringCompare(Line, "v ", 2))
        {
            ++VerticesCount;
        }
        if(StringCompare(Line, "vt ", 3))
        {
            ++TextureCoordsCount; 
        }
        if(StringCompare(Line, "vn ", 3))
        {
            ++NormalsCount;
        }
        if(StringCompare(Line, "f ", 2))
        {
            ++IndexCount;
        }    
        
        int Counter = 0;
        char *Letter = Line;
        while(*Letter != '\n')
        {
            ++Counter;
            ++Letter;
        }
        Line += Counter + 1;
    }
    
    // get the memory
    float *TempVertices = (float *)PlatformAllocMemory((VerticesCount*3)*sizeof(float));
    float *TempTextureCoords = (float *)PlatformAllocMemory((TextureCoordsCount*2)*sizeof(float));
    float *TempNormals = (float *)PlatformAllocMemory((NormalsCount*3)*sizeof(float));
    int *TempVIndex = (int *)PlatformAllocMemory((IndexCount*3)*sizeof(int));
    int *TempTIndex = (int *)PlatformAllocMemory((IndexCount*3)*sizeof(int));
    int *TempNIndex = (int *)PlatformAllocMemory((IndexCount*3)*sizeof(int));
    
    // second loop through the file to ge the values 
    float *VerticesPtr = TempVertices;
    float *TextureCoordsPtr = TempTextureCoords;
    float *NormalsPtr = TempNormals;
    int *VIndexPtr = TempVIndex;
    int *TIndexPtr = TempTIndex;
    int *NIndexPtr = TempNIndex;
    Line = (char *)OBJFile;
    while(*Line != '\0')
    {   
        if(StringCompare(Line, "v ", 2))
        {
            float X, Y, Z;
            sscanf(Line, "v %f %f %f", &X, &Y, &Z);
            *VerticesPtr++ = X;
            *VerticesPtr++ = Y;
            *VerticesPtr++ = Z;
        }
        if(StringCompare(Line, "vt ", 3))
        {
            float U, V;
            sscanf(Line, "vt %f %f", &U, &V);
            *TextureCoordsPtr++ = U;
            *TextureCoordsPtr++ = V;
        }
        if(StringCompare(Line, "vn ", 3))
        {
            float X, Y, Z;
            sscanf(Line, "vn %f %f %f", &X, &Y, &Z);
            *NormalsPtr++ = X;
            *NormalsPtr++ = Y;
            *NormalsPtr++ = Z;
        }
        if(StringCompare(Line, "f ", 2))
        {
            int VIndex[3];
            int TIndex[3];
            int NIndex[3];
            sscanf(Line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                   &VIndex[0], &TIndex[0], &NIndex[0],
                   &VIndex[1], &TIndex[1], &NIndex[1],
                   &VIndex[2], &TIndex[2], &NIndex[2]);
            *VIndexPtr++ = VIndex[0] - 1;
            *VIndexPtr++ = VIndex[1] - 1;
            *VIndexPtr++ = VIndex[2] - 1;
            *TIndexPtr++ = TIndex[0] - 1;
            *TIndexPtr++ = TIndex[1] - 1;
            *TIndexPtr++ = TIndex[2] - 1;
            *NIndexPtr++ = NIndex[0] - 1;
            *NIndexPtr++ = NIndex[1] - 1;
            *NIndexPtr++ = NIndex[2] - 1;
        }  

        int Counter = 0;
        char *Letter = Line;
        while(*Letter != '\n')
        {
            ++Counter;
            ++Letter;
        }
        Line += Counter + 1;
    }

    // this is store in the permanent storage of the program  
    Result.Vertices = (float *)PushArray(Arena, (3*8*IndexCount), float);
    float *VertexBufferPtr = Result.Vertices;
    VIndexPtr = TempVIndex;
    TIndexPtr = TempTIndex;
    NIndexPtr = TempNIndex;
    for(int Index = 0;
        Index < 3*IndexCount;
        ++Index)
    {
        int VIndex = *VIndexPtr * 3;
        *VertexBufferPtr++ = TempVertices[VIndex + 0];
        *VertexBufferPtr++ = TempVertices[VIndex + 1];
        *VertexBufferPtr++ = TempVertices[VIndex + 2];
        ++VIndexPtr;
        int TIndex = *TIndexPtr * 2;
        *VertexBufferPtr++ = TempTextureCoords[TIndex + 0];
        *VertexBufferPtr++ = TempTextureCoords[TIndex + 1];
        ++TIndexPtr;
        int NIndex = *NIndexPtr * 3;
        *VertexBufferPtr++ = TempNormals[NIndex + 0];
        *VertexBufferPtr++ = TempNormals[NIndex + 1];
        *VertexBufferPtr++ = TempNormals[NIndex + 2];
        ++NIndexPtr;
    }
    
    Result.VerticesCount = 3*8*IndexCount;

    PlatformFreeMemory(TempVertices);
    PlatformFreeMemory(TempTextureCoords);
    PlatformFreeMemory(TempNormals);
    PlatformFreeMemory(TempVIndex);
    PlatformFreeMemory(TempTIndex);
    PlatformFreeMemory(TempNIndex);
    //PlatformFreeMemory(OBJFile);

    return Result;
}

