static obj
LoadOBJFile(char *OBJFileName, arena *Arena)
{
    obj Result = {};
    void* OBJFile = ReadEntireFile(OBJFileName, NULL, Arena);
    if(!OBJFile)
    {
        OutputDebugString("ERROR::LOADING::OBJ\n");
        return Result;
    }
    
    int VerticesCount = 0; 

    // loop the file by line
    char *Line = (char *)OBJFile;
    while(*Line != '\0')
    {
        
         
        if(strncmp(Line, "v ", 2) == 0)
        {
            ++VerticesCount;
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
    int StopHere = 0;
    return Result;
}

/*
void LoadOBJFileIndex(Mesh* mesh, const char* filePhat, const char* texFileName)
{
    mesh->numVertices  = 0;
    mesh->numIndex     = 0;
    mesh->numTexCoords = 0;
    mesh->numNormals   = 0;

    FILE* file;
    file = fopen(filePhat, "r");

    if(file == NULL)
    {
        OutputDebugString("ERROR::LOADING::OBJ::FILE\n");
    }
    char line[1024];

    while(fgets(line, 1024, file) != NULL)
    {    
        // first we have to count the size 
        // becouse we need to allocate memory for
        // the obj object 
        if(strncmp(line, "v ", 2) == 0)
        {
            mesh->numVertices++;
        }
        if(strncmp(line, "vt ", 3) == 0)
        {
            mesh->numTexCoords++;
        }
        if(strncmp(line, "vn ", 3) == 0)
        {
            mesh->numNormals++;
        }
        if(strncmp(line, "f ", 2) == 0)
        {
            mesh->numIndex++;
        }
    } 
    // we allocate memory for the model 
    mesh->vertices      = (Vec3*)malloc(mesh->numVertices  * sizeof(Vec3));
    mesh->textureCoords = (Vec2*)malloc(mesh->numTexCoords * sizeof(Vec2));
    mesh->normals       = (Vec3*)malloc(mesh->numNormals   * sizeof(Vec3));
    mesh->vertexBuffer  = (VertexBuffer*)malloc((mesh->numIndex * 3) * sizeof(VertexBuffer));
    mesh->fVertexBuffer = (VertexBuffer*)malloc(mesh->numVertices * sizeof(VertexBuffer));
    mesh->vertexIndex   = (IndexBuffer*)malloc(mesh->numIndex * sizeof(IndexBuffer));
    mesh->textureIndex  = (IndexBuffer* )malloc(mesh->numIndex * sizeof(IndexBuffer));
    mesh->normalIndex   = (IndexBuffer*)malloc(mesh->numIndex * sizeof(IndexBuffer));
    mesh->indices       = (int*)malloc((mesh->numIndex * 3) * sizeof(int));
    
    Vec3 test;
    rewind(file);
    int vertexIndex = 0;
    int textIndex = 0;
    int normalIndex = 0;
    int indexIndex = 0;
    while(fgets(line, 1024, file) != NULL)
    {     
        if(strncmp(line, "v ", 2) == 0)
        {
            float x, y, z;
            sscanf(line, "v %f %f %f", &x, &y, &z);
            mesh->vertices[vertexIndex].x = x;
            mesh->vertices[vertexIndex].y = y;
            mesh->vertices[vertexIndex].z = z; 
            vertexIndex++;
        }
        if(strncmp(line, "vt ", 3) == 0)
        {
            float x, y;
            sscanf(line, "vt %f %f", &x, &y);
            mesh->textureCoords[textIndex].x = x;
            mesh->textureCoords[textIndex].y = y;
            textIndex++;
        }
        if(strncmp(line, "vn ", 3) == 0)
        {
            float x, y, z;
            sscanf(line, "vn %f %f %f", &x, &y, &z);
            mesh->normals[normalIndex].x = x;
            mesh->normals[normalIndex].y = y;
            mesh->normals[normalIndex].z = z;
            normalIndex++;
        }
        if(strncmp(line, "f ", 2) == 0)
        {
            int indices[3];
            int textures[3];
            int normals[3];
            sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                &indices[0], &textures[0], &normals[0],
                &indices[1], &textures[1], &normals[1],
                &indices[2], &textures[2], &normals[2]);

            mesh->vertexIndex[indexIndex].a = indices[0];
            mesh->vertexIndex[indexIndex].b = indices[1];
            mesh->vertexIndex[indexIndex].c = indices[2];

            mesh->textureIndex[indexIndex].a = textures[0];
            mesh->textureIndex[indexIndex].b = textures[1];
            mesh->textureIndex[indexIndex].c = textures[2];

            mesh->normalIndex[indexIndex].a = normals[0];
            mesh->normalIndex[indexIndex].b = normals[1];
            mesh->normalIndex[indexIndex].c = normals[2];
            indexIndex++;
        }
    }
    
    // with all the obj info we have to fill the vertex and index buffer
    int indexCounter = 0;
    for(int i = 0; i < (mesh->numIndex * 3); i += 3)
    {
        // vertice
        mesh->vertexBuffer[i].vertice.x = mesh->vertices[mesh->vertexIndex[indexCounter].a - 1].x;
        mesh->vertexBuffer[i].vertice.y = mesh->vertices[mesh->vertexIndex[indexCounter].a - 1].y;
        mesh->vertexBuffer[i].vertice.z = mesh->vertices[mesh->vertexIndex[indexCounter].a - 1].z;
        mesh->vertexBuffer[i + 1].vertice.x = mesh->vertices[mesh->vertexIndex[indexCounter].b - 1].x;
        mesh->vertexBuffer[i + 1].vertice.y = mesh->vertices[mesh->vertexIndex[indexCounter].b - 1].y;
        mesh->vertexBuffer[i + 1].vertice.z = mesh->vertices[mesh->vertexIndex[indexCounter].b - 1].z;
        mesh->vertexBuffer[i + 2].vertice.x = mesh->vertices[mesh->vertexIndex[indexCounter].c - 1].x;
        mesh->vertexBuffer[i + 2].vertice.y = mesh->vertices[mesh->vertexIndex[indexCounter].c - 1].y;
        mesh->vertexBuffer[i + 2].vertice.z = mesh->vertices[mesh->vertexIndex[indexCounter].c - 1].z;
        // texture Coords
        mesh->vertexBuffer[i].textureCoord.x = mesh->textureCoords[mesh->textureIndex[indexCounter].a - 1].x;
        mesh->vertexBuffer[i].textureCoord.y = mesh->textureCoords[mesh->textureIndex[indexCounter].a - 1].y;
        mesh->vertexBuffer[i + 1].textureCoord.x = mesh->textureCoords[mesh->textureIndex[indexCounter].b - 1].x;
        mesh->vertexBuffer[i + 1].textureCoord.y = mesh->textureCoords[mesh->textureIndex[indexCounter].b - 1].y;
        mesh->vertexBuffer[i + 2].textureCoord.x = mesh->textureCoords[mesh->textureIndex[indexCounter].c - 1].x;
        mesh->vertexBuffer[i + 2].textureCoord.y = mesh->textureCoords[mesh->textureIndex[indexCounter].c - 1].y;
        // normals
        mesh->vertexBuffer[i].normal.x = mesh->normals[mesh->normalIndex[indexCounter].a - 1].x;
        mesh->vertexBuffer[i].normal.y = mesh->normals[mesh->normalIndex[indexCounter].a - 1].y;
        mesh->vertexBuffer[i].normal.z = mesh->normals[mesh->normalIndex[indexCounter].a - 1].z;
        mesh->vertexBuffer[i + 1].normal.x = mesh->normals[mesh->normalIndex[indexCounter].b - 1].x;
        mesh->vertexBuffer[i + 1].normal.y = mesh->normals[mesh->normalIndex[indexCounter].b - 1].y;
        mesh->vertexBuffer[i + 1].normal.z = mesh->normals[mesh->normalIndex[indexCounter].b - 1].z;
        mesh->vertexBuffer[i + 2].normal.x = mesh->normals[mesh->normalIndex[indexCounter].c - 1].x;
        mesh->vertexBuffer[i + 2].normal.y = mesh->normals[mesh->normalIndex[indexCounter].c - 1].y;
        mesh->vertexBuffer[i + 2].normal.z = mesh->normals[mesh->normalIndex[indexCounter].c - 1].z;
        indexCounter++;
    }

    for(int i = 0; i < mesh->numIndex; i++)
    {
        int index = i * 3;
        mesh->indices[index + 0] = mesh->vertexIndex[i].a - 1;
        mesh->indices[index + 1] = mesh->vertexIndex[i].b - 1;
        mesh->indices[index + 2] = mesh->vertexIndex[i].c - 1;
    }

    for(int i = 0; i < mesh->numIndex * 3; i++)
    {
        mesh->fVertexBuffer[mesh->indices[i]] = mesh->vertexBuffer[i];
    }

    glGenVertexArrays(1, &mesh->vao);
    glBindVertexArray(mesh->vao);
    
    uint32_t verticesVBO;
    glGenBuffers(1, &verticesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, verticesVBO);
    glBufferData(GL_ARRAY_BUFFER, mesh->numVertices*sizeof(VertexBuffer), mesh->fVertexBuffer, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexBuffer), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexBuffer), (void*)(3 * sizeof(float)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBuffer), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    uint32_t EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (mesh->numIndex * 3) * sizeof(int), mesh->indices, GL_STATIC_DRAW);

    free(mesh->vertices); 
    free(mesh->textureCoords);
    free(mesh->normals); 
    free(mesh->vertexBuffer);
    free(mesh->vertexIndex);
    free(mesh->textureIndex);
    free(mesh->normalIndex);
    free(mesh->fVertexBuffer);
    free(mesh->indices);
 
    uint32_t texture1;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    mesh->texId = texture1;
    // test loadd bmp file:
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    mesh->tex = LoadBMP(texFileName);

    if(mesh->tex.pixels != NULL)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mesh->tex.width, mesh->tex.height,
                                    0, GL_BGRA, GL_UNSIGNED_BYTE, mesh->tex.pixels);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        OutputDebugString("ERROR::LOADING::BMP::FILE\n");
    }
    free(mesh->tex.pixels);

    mesh->model = get_identity_matrix();
}
*/
