static void *
ReadEntireFile(char *FileName, SIZE_T *FileSizePtr, arena *Arena)
{
    HANDLE FileHandle =  CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, 0,
                                     OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if(FileHandle == INVALID_HANDLE_VALUE)
    {
        OutputDebugString("ERROR::OPENING::FILE\n");
        return 0;
    }
    LARGE_INTEGER FileSize;
    GetFileSizeEx(FileHandle, &FileSize);
    void *FileBuffer = PushSize(Arena, FileSize.QuadPart);
    if(FileSizePtr) *FileSizePtr = (SIZE_T)FileSize.QuadPart;
    if(!ReadFile(FileHandle, FileBuffer, (DWORD)FileSize.QuadPart, 0, 0))
    {
        OutputDebugString("ERROR::READING::FILE\n");
        return 0;
    }
    return FileBuffer;
}

static bit_map
LoadBMP(char *FileName, arena *Arena)
{
    bit_map Result = {};   
    void *BMPFile = ReadEntireFile(FileName, &Result.Size, Arena);
    if(!BMPFile)
    {
        OutputDebugString("ERROR::LOADING::BMP\n");
        return Result;
    }

    bit_map_header *BMPHeader = (bit_map_header *)BMPFile;

    Result.Width = BMPHeader->Width;
    Result.Height = BMPHeader->Height;
    Result.BitsPerPixel = BMPHeader->BitsPerPixel;

    if(Result.BitsPerPixel == 8)
    {
        Result.ColorPalette = (unsigned int *)((unsigned char *)BMPFile + sizeof(bit_map_header));
        int StopHere = 0;
    }
    Result.Pixels = (void *)((unsigned char *)BMPFile + BMPHeader->BitmapOffset);

    return Result;
}
