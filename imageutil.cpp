#include <windows.h>
#include <stdio.h>
#include "imageutil.h"

// Returns a pointer to a specified section.
// (returns NULL if section was such name was not found)
IMAGE_SECTION_HEADER* GetSectionHeader(char* name)
{
	HANDLE hMod = GetModuleHandle(NULL);
	IMAGE_DOS_HEADER* p = (IMAGE_DOS_HEADER*)hMod;
	IMAGE_NT_HEADERS* nth = (IMAGE_NT_HEADERS*)((BYTE*)hMod + p->e_lfanew);
	IMAGE_FILE_HEADER* fh = &(nth->FileHeader);
	IMAGE_SECTION_HEADER* sec = 
		(IMAGE_SECTION_HEADER*)((BYTE*)fh + sizeof(IMAGE_FILE_HEADER) + fh->SizeOfOptionalHeader);

	WORD num = fh->NumberOfSections;
	for (WORD i=0; i<num; i++)
	{
		if (strncmp((char*)sec[i].Name, name, 8)==0) return sec + i;
	}
	return NULL;
}

// Returns a pointer to a specified section.
// (returns NULL if section was such name was not found)
IMAGE_SECTION_HEADER* GetSectionHeaderByOrdinal(int i)
{
	HANDLE hMod = GetModuleHandle(NULL);
	IMAGE_DOS_HEADER* p = (IMAGE_DOS_HEADER*)hMod;
	IMAGE_NT_HEADERS* nth = (IMAGE_NT_HEADERS*)((BYTE*)hMod + p->e_lfanew);
	IMAGE_FILE_HEADER* fh = &(nth->FileHeader);
	IMAGE_SECTION_HEADER* sec =
		(IMAGE_SECTION_HEADER*)((BYTE*)fh + sizeof(IMAGE_FILE_HEADER) + fh->SizeOfOptionalHeader);

	WORD num = fh->NumberOfSections;
    if (i>=0 && i<num) {
        return sec + i;
    }
	return NULL;
}

// Initializes the output "ppDataDirectory" parameter with a pointer to 
// IMAGE_DATA_DIRECTORY structure. Returns the number of directory
// entries.(0 - if module has no optional header)
DWORD GetImageDataDirectory(HMODULE hMod, IMAGE_DATA_DIRECTORY** ppDataDirectory)
{
	IMAGE_DOS_HEADER* p = (IMAGE_DOS_HEADER*)hMod;
	IMAGE_NT_HEADERS* nth = (IMAGE_NT_HEADERS*)((DWORD)hMod + p->e_lfanew);
	IMAGE_FILE_HEADER* fh = &(nth->FileHeader);
	if (fh->SizeOfOptionalHeader == 0) return 0;

	IMAGE_OPTIONAL_HEADER* oh = &(nth->OptionalHeader);
	*ppDataDirectory = oh->DataDirectory;
	return oh->NumberOfRvaAndSizes;
}

// Returns the pointer to the first IMAGE_IMPORT_DESCRIPTOR structure.
// If moduleName is NULL, the main application is assumed (as opposed to DLL).
// Returns NULL if no optional header exists, or if no import descriptors)
IMAGE_IMPORT_DESCRIPTOR* GetImageImportDescriptors(char* moduleName)
{
	HMODULE hMod = GetModuleHandle(moduleName);
	return GetModuleImportDescriptors(hMod);
}

// Returns the pointer to the first IMAGE_IMPORT_DESCRIPTOR structure.
// (returns NULL if no optional header exists, or if no import descriptors)
IMAGE_IMPORT_DESCRIPTOR* GetModuleImportDescriptors(HMODULE hMod)
{
	IMAGE_DATA_DIRECTORY* dataDirectory = NULL;
	DWORD numEntries = GetImageDataDirectory(hMod, &dataDirectory);
	if (numEntries < 2) return NULL;
	DWORD rva = dataDirectory[1].VirtualAddress;
	return (IMAGE_IMPORT_DESCRIPTOR*)((DWORD)hMod + (DWORD)rva);
}

// Position the file at the beginning of specified section
// Returns true if positioning was successful.
bool SeekSectionHeader(FILE* f, char* name)
{
	fseek(f, 0, SEEK_SET);
	IMAGE_DOS_HEADER dh;
	fread(&dh, sizeof(IMAGE_DOS_HEADER), 1, f);
	fseek(f, dh.e_lfanew - sizeof(IMAGE_DOS_HEADER), SEEK_CUR);
	IMAGE_NT_HEADERS nth;
	fread(&nth, sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER), 1, f);
	fseek(f, nth.FileHeader.SizeOfOptionalHeader, SEEK_CUR);

	IMAGE_SECTION_HEADER sec;
	WORD num = nth.FileHeader.NumberOfSections;
	for (WORD i=0; i<num; i++)
	{
		fread(&sec, sizeof(IMAGE_SECTION_HEADER), 1, f);
		if (strlen(name)==0 || memcmp(sec.Name, name, 8)==0 || strncmp((char*)sec.Name, name, 8)==0)
		{
			// go back the length of the section, because
			// we want to position at the beginning
			fseek(f, -sizeof(IMAGE_SECTION_HEADER), SEEK_CUR);
			return true;
		}
	}
	return false;
}

// Position the file at the beginning of section virtual address
// Returns true if positioning was successful.
bool SeekSectionVA(FILE* f, char* name)
{
	if (SeekSectionHeader(f, name))
	{
		fseek(f, sizeof(BYTE)*IMAGE_SIZEOF_SHORT_NAME, SEEK_CUR); // section name
		fseek(f, sizeof(DWORD), SEEK_CUR); // misc union
		return true;
	}
	return false;
}

// Position the file at the entry point address in PE-header
// Returns true if positioning was successful
bool SeekEntryPoint(FILE* f)
{
	fseek(f, 0, SEEK_SET);
	IMAGE_DOS_HEADER dh;
	fread(&dh, sizeof(IMAGE_DOS_HEADER), 1, f);
	fseek(f, dh.e_lfanew - sizeof(IMAGE_DOS_HEADER), SEEK_CUR);
	IMAGE_NT_HEADERS nth;
	fread(&nth, sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER), 1, f);
	if (nth.FileHeader.SizeOfOptionalHeader > 0)
	{
		fseek(f, sizeof(WORD), SEEK_CUR);  // magic
		fseek(f, sizeof(BYTE), SEEK_CUR);  // major linker version
		fseek(f, sizeof(BYTE), SEEK_CUR);  // minor linker version
		fseek(f, sizeof(DWORD), SEEK_CUR); // size of code
		fseek(f, sizeof(DWORD), SEEK_CUR); // size of initialized data
		fseek(f, sizeof(DWORD), SEEK_CUR); // size of uninitialized data
		return true;
	}
	return false;
}

// Position the file at the image base address in PE-header
// Returns true if positioning was successful
bool SeekImageBase(FILE* f)
{
	if (SeekEntryPoint(f))
	{
		fseek(f, sizeof(DWORD), SEEK_CUR); // address of entry point
		fseek(f, sizeof(DWORD), SEEK_CUR); // base of code
		fseek(f, sizeof(DWORD), SEEK_CUR); // base of data
		return true;
	}
	return false;
}

DWORD getFileOffset(FILE *f, DWORD RVA)
{
	fseek(f, 0, SEEK_SET);
	IMAGE_DOS_HEADER dh;
	fread(&dh, sizeof(IMAGE_DOS_HEADER), 1, f);
	fseek(f, dh.e_lfanew - sizeof(IMAGE_DOS_HEADER), SEEK_CUR);
	IMAGE_NT_HEADERS nth;
	fread(&nth, sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER), 1, f);
	fseek(f, nth.FileHeader.SizeOfOptionalHeader, SEEK_CUR);

	IMAGE_SECTION_HEADER sec;
	WORD num = nth.FileHeader.NumberOfSections;
	for (WORD i=0; i<num; i++)
	{
		fread(&sec, sizeof(IMAGE_SECTION_HEADER), 1, f);
		if (RVA >= sec.VirtualAddress && RVA < (sec.VirtualAddress + sec.Misc.VirtualSize)) {
			return RVA - sec.VirtualAddress + sec.PointerToRawData;
		}
	}
	
	return 0;
}

DWORD SeekImportTable(FILE *f)
{
	DWORD itRVA=0, itSize=0;
	
	fseek(f, 0, SEEK_SET);
	IMAGE_DOS_HEADER dh;
	fread(&dh, sizeof(IMAGE_DOS_HEADER), 1, f);
	fseek(f, dh.e_lfanew - sizeof(IMAGE_DOS_HEADER), SEEK_CUR);
	IMAGE_NT_HEADERS nth;
	fread(&nth, sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER), 1, f);
	if (nth.FileHeader.SizeOfOptionalHeader > 0)
	{
		fseek(f, 104, SEEK_CUR);
		fread(&itRVA, sizeof(DWORD), 1, f);
		fread(&itSize, sizeof(DWORD), 1, f);
		fseek(f, getFileOffset(f, itRVA), SEEK_SET);

		return itSize;
	}	
	return 0;
}

DWORD getImportThunkRVA(FILE* f, char* dllName, char* funcName)
{
	if (strlen(dllName)==0 || strlen(funcName)==0) return 0;
	
	DWORD itSize = SeekImportTable(f);
	DWORD numDLLs = itSize / sizeof(IMAGE_IMPORT_DESCRIPTOR);
	
	for (WORD i=0; i < numDLLs; i++)
	{
		IMAGE_IMPORT_DESCRIPTOR iid;
		fread(&iid, sizeof(IMAGE_IMPORT_DESCRIPTOR), 1, f);
		DWORD fp = ftell(f);

		char iidName[256];
		fseek(f, getFileOffset(f, iid.Name), SEEK_SET);
		fread(iidName, 256, 1, f);
		
		//find the right dll
		if (stricmp(iidName, dllName) == 0) {
			// the FLT no-dvd has no OFT, but for the vitality no-dvd the FT is wrong
			if (iid.OriginalFirstThunk)
				fseek(f, getFileOffset(f, iid.OriginalFirstThunk), SEEK_SET);
			else
				fseek(f, getFileOffset(f, iid.FirstThunk), SEEK_SET);
			
			//now find the function
			for (WORD j=0;; j++)
			{
				DWORD thunkRVA;
				fread(&thunkRVA, sizeof(DWORD), 1, f);
				DWORD fp1 = ftell(f);
				if (thunkRVA==0) break;
				
				char funcName1[256];
				fseek(f, getFileOffset(f, thunkRVA) + sizeof(WORD), SEEK_SET);
				fread(funcName1, 256, 1, f);
				
				if (stricmp(funcName1, funcName) == 0) {
					return iid.FirstThunk + j*sizeof(DWORD);
				}
				
				fseek(f, fp1, SEEK_SET);
			}
		}
		
		fseek(f, fp, SEEK_SET);
	}

	return 0;
}

// Returns a pointer to code section header
IMAGE_SECTION_HEADER* GetCodeSectionHeader()
{
	return GetSectionHeader(".text");
}

// Position the file at the code section flags. 
// Returns true if positioning was successful.
bool SeekCodeSectionFlags(FILE* f)
{
	if (SeekSectionHeader(f, ".text"))
	{
		// read the section except for the last 4 bytes,
		// so that we're positioned at the section flags
		fseek(f, sizeof(IMAGE_SECTION_HEADER) - 4, SEEK_CUR);
		return true;
	}
	return false;
}

