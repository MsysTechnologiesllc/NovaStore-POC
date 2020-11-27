#include<stdio.h>
#include<Windows.h>

#define BUFFERSIZE 512

struct offset { //demo structure
    int startOffset;
    int length;
};

void inSourceBackupFileName(char full_path[], char attchPath[], char* ptr) { /// have to be replace before commiting file

    char path[_MAX_PATH] = { '\0' };
    char drive[_MAX_DRIVE] = { '\0' };
    char dir[_MAX_DIR] = { '\0' };
    char fname[_MAX_FNAME] = { '\0' };
    char ext[_MAX_EXT] = { '\0' };

    _splitpath_s(full_path, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);

    strcat_s(path, _MAX_FNAME, fname);
    strcat_s(path, _MAX_FNAME, attchPath);

    if (strlen(ext) != 0) {
        strcat_s(path, _MAX_FNAME, ext);
    }

    printf("file name is %s\n", path);
    strcpy_s(ptr, MAX_PATH, path);
}

void writestruct(char sourceFilePath[], struct offset objArr[]) {


    char dFileName[MAX_PATH] = "D:\\dataFilex";
    char metaDataFile[] = "_metaData";
    char DataFile[] = "_Data";
    //char buffer[4096] = "CreateFile 1234567890 was originally developed specifically for file interaction but has since been expanded and enhanced to include most other types of I/O devices and mechanisms available to Windows developers. This section attempts to cover the varied issues developers may experience when using CreateFile in different contexts and with different I/O types. The text attempts to use the word file only when referring specifically to data stored in an actual file on a file system. However, some uses of file may be referring more generally to an I/O object that supports file-like mechanisms. This liberal use of the term file is particularly prevalent in constant names and parameter names because of the previously mentioned historical reasons.When an application is finished using the object handle returned by CreateFile, use the CloseHandle function to close the handle.This not only frees up system resources, but can have wider influence on things like sharing the file or device and committing data to disk.Specifics are noted within this topic as appropriate.";
    char mfilePath[_MAX_PATH] = { '\0' };
    char dfilePath[_MAX_PATH] = { '\0' };
    char buffer[15] = "1234567890";
    char ReadBuffer[BUFFERSIZE] = { '\0' };

    HANDLE HdestiFile;

    FILE* outfile;

    //*********open metaData  file for & write struct into that file 

    inSourceBackupFileName(sourceFilePath, metaDataFile, mfilePath);  //call to  create name of metadata file name 
    fopen_s(&outfile, mfilePath, "w");
    if (outfile == NULL)
    {
        fprintf(stderr, "\nError opend file\n");
        exit(1);
    }
    for (int i = 0; i < 5; i++) {

        fwrite(&objArr[i], sizeof(struct offset), 1, outfile);

        if (fwrite != 0)
            printf("contents to file written successfully !\n");
        else
            printf("error writing file !\n");

    }
    fclose(outfile); // close file


    // *********create sparse file  for writing data in it

    DWORD dwTemp;
    inSourceBackupFileName(sourceFilePath, DataFile, dfilePath); //call to  create name of data file name 
    size_t newDestiSize = strlen(dfilePath) + 1;
    wchar_t* wcDestin = new wchar_t[newDestiSize];
    size_t convertedDestiChars = 0;
    mbstowcs_s(&convertedDestiChars, wcDestin, newDestiSize, dfilePath, _TRUNCATE);

    HANDLE hSparseFile = CreateFile(wcDestin, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hSparseFile == INVALID_HANDLE_VALUE)
        printf("INVALID_HANDLE_VALUE \n");

    BOOL Status = DeviceIoControl(hSparseFile, FSCTL_SET_SPARSE, NULL, 0, NULL, 0, &dwTemp, NULL); // make created file as sparse file 

    if (Status) {
        printf("successfull \n");
    }
    else {
        printf("error in deviceIOcontrol ", GetLastError());
        return; //return the error value
    }


    // ****** write data into file 

    DWORD wCurrentFilePosition = SetFilePointer(hSparseFile, 1073741824, NULL, FILE_BEGIN);
    for (int j = 0; j < 5; j++) {

        DWORD dwCurrentFilePosition = SetFilePointer(hSparseFile, objArr[j].startOffset, NULL, FILE_CURRENT);
        printf("Current file pointer position: %d\n", dwCurrentFilePosition);

        printf(" buffer data are %d\n", strlen(buffer));
        if (!WriteFile(hSparseFile, buffer, strlen(buffer), NULL, NULL)) {
            printf("failed to write\n");
            return;
        }
    }
    printf(" write  successfull\n");
    


    // *******retrive data of  mataData file for restore purpose

    FILE* infile;
    struct offset input;

    // Open person.dat for reading
    fopen_s(&infile, mfilePath, "r");
    if (infile == NULL)
    {
        fprintf(stderr, "\nError opening file\n");
        exit(1);
    }

   
    // read file contents till end of file and print 
    printf("reading data file \n\n");
    SetFilePointer(hSparseFile, 1073741824, NULL, FILE_BEGIN);
    while (fread(&input, sizeof(struct offset), 1, infile)) {
        printf("input  startOffset %d\n        ", input.startOffset);
        SetFilePointer(hSparseFile, input.startOffset, NULL, FILE_CURRENT);
        if (!ReadFile(hSparseFile, ReadBuffer, 10, NULL, NULL)) {
            printf("failed to read file \n");
        }
        else {
            printf("read file data are %s\n\n", ReadBuffer);

        }
    }

    RtlZeroMemory(ReadBuffer, 10); ///zeroOut Buffer 

    fclose(infile);


    printf("Done\n");

}


int main() {
    char fileArr[] = "D:\\backupFile.dat";

    struct offset objArr[5]; // array of structure

    objArr[0].startOffset = 100;
    objArr[0].length = 10;

    objArr[1].startOffset = 200;
    objArr[1].length = 10;

    objArr[2].startOffset = 300;
    objArr[2].length = 10;

    objArr[3].startOffset = 400;
    objArr[3].length = 10;

    objArr[4].startOffset = 500;
    objArr[4].length = 10;

    writestruct(fileArr, objArr);
    return 0;
}




