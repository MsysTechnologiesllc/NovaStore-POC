
#include<vss.h>
#include<iostream>
#include<vswriter.h>
#include<vsbackup.h>
#include <comutil.h>
#include<tchar.h>
#include <atlstr.h>
#pragma warning(disable:4996)
#include <vector>
#include <strsafe.h>
#include <string>
#include <cstdlib>
#include <windows.h>
#include <winioctl.h>
#include <virtdisk.h>
#include <initguid.h>
#pragma comment(lib, "virtdisk.lib")
using namespace std;



#define BUFFERSIZE 4096
bool toggleRestore = false;
char backUpFile[] = "C:\\BackupData";




DEFINE_GUID(VIRTUAL_STORAGE_TYPE_VENDOR_MICROSOFT, 0xec984aec, 0xa0f9, 0x47e9, 0x90, 0x1f, 0x71, 0x41, 0x5a, 0x66, 0x34, 0x5b);
DEFINE_GUID(VIRTUAL_STORAGE_TYPE_VENDOR_UNKNOWN, 0x00000000, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

int activate_rct(string gpath)
{
	HANDLE vhdHandle;
	_VIRTUAL_STORAGE_TYPE storageType;
	storageType.DeviceId = VIRTUAL_STORAGE_TYPE_DEVICE_UNKNOWN;
	storageType.VendorId = { 0xec984aec, 0xa0f9, 0x47e9, {0x90, 0x1f, 0x71, 0x41, 0x5a, 0x66, 0x34, 0x5b} };
	wchar_t* path = new wchar_t[gpath.length() + 1];
	std::copy(gpath.begin(), gpath.end(), path);
	VIRTUAL_DISK_ACCESS_MASK mask = VIRTUAL_DISK_ACCESS_ALL;
	PSET_VIRTUAL_DISK_INFO diskInfo;
	ULONG diskInfoSize = sizeof(SET_VIRTUAL_DISK_INFO);
	std::wcout << "size of diskinfo structure " << diskInfoSize << std::endl;
	diskInfo = (PSET_VIRTUAL_DISK_INFO)malloc(diskInfoSize);
	if (diskInfo == NULL)
	{
		std::cout << "Failed to malloc disk info, ret=" << std::endl;
		return 0;
	}
	std::wcout << "Opening Virtual disk " << path << std::endl;
	DWORD res = OpenVirtualDisk(&storageType, path, mask, OPEN_VIRTUAL_DISK_FLAG_NONE, NULL, &vhdHandle);

	if (res != ERROR_SUCCESS)
	{
		std::cout << "Failed to open disk, ret=" << res << std::endl;
		return 0;
	}
	diskInfo->Version = SET_VIRTUAL_DISK_INFO_CHANGE_TRACKING_STATE;
	diskInfo->ChangeTrackingEnabled = true;
	res = SetVirtualDiskInformation(vhdHandle, diskInfo);
	return res;
}


void BackupFileName(char full_path[], char desti_path[], char* ptr, int no) {

	char path[_MAX_PATH] = { '\0' };
	char drive[_MAX_DRIVE] = { '\0' };
	char dir[_MAX_DIR] = { '\0' };
	char fname[_MAX_FNAME] = { '\0' };
	char ext[_MAX_EXT] = { '\0' };

	_splitpath_s(full_path, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);

	strcat_s(path, MAX_PATH, desti_path);
	if (no == 1) {
		strcat_s(path, _MAX_FNAME, dir);//

	}
	else {
		strcat_s(path, MAX_PATH, "\\");
	}
	strcat_s(path, _MAX_FNAME, fname);

	if (strlen(ext) != 0) {

		strcat_s(path, _MAX_FNAME, ext);
	}

	strcpy_s(ptr, MAX_PATH, path);

}

void sourceBackupFileName(char full_path[], char desti_path[], char* ptr, bool restore) {

	char path[_MAX_PATH] = { '\0' };
	char restoreFile[_MAX_PATH] = { '\0' };
	char drive[_MAX_DRIVE] = { '\0' };
	char dir[_MAX_DIR] = { '\0' };
	char fname[_MAX_FNAME] = { '\0' };
	char ext[_MAX_EXT] = { '\0' };


	_splitpath_s(full_path, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);

	if (restore == false) {
		int lastIndex = (strlen(desti_path) - 1);
		strcat_s(path, MAX_PATH, desti_path);
		if (desti_path[lastIndex] != '\\')
			strcat_s(path, MAX_PATH, "\\");
		strcat_s(path, _MAX_FNAME, fname);
	}
	if (strlen(ext) != 0) {

		strcat_s(path, _MAX_FNAME, ext);
	}

	if (restore == true) {
		strcat_s(restoreFile, _MAX_FNAME, fname);
		strcat_s(restoreFile, _MAX_FNAME, ext);
		strcpy_s(ptr, MAX_PATH, restoreFile);
	}
	else {
		strcpy_s(ptr, MAX_PATH, path);
	}

}


void destiBackupFileName(char full_path[], char desti_path[], char* ptr) {


	HANDLE HdestiDir;
	char path[_MAX_PATH] = { '\0' };
	char drive[_MAX_DRIVE] = { '\0' };
	char dir[_MAX_DIR] = { '\0' };
	char lastDir[MAX_PATH] = { '\0' };
	char fname[_MAX_FNAME] = { '\0' };
	char ext[_MAX_EXT] = { '\0' };

	_splitpath_s(full_path, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);

	strcat_s(path, MAX_PATH, desti_path);

	strcat_s(path, MAX_PATH, "\\");
	strcat_s(path, _MAX_FNAME, fname);

	if (strlen(ext) != 0) {
		strcat_s(path, _MAX_FNAME, ext);
	}

	strcpy_s(ptr, MAX_PATH, path);
	//printf("copy file name is %s\n", path);
}

bool MetadatModify(HANDLE hFileOpen, HANDLE hFileCreate) {

	/////******************** for FileBasicInfo *********************************************** //////

	size_t size = sizeof(FILE_BASIC_INFO);
	FILE_BASIC_INFO* fileBasicInformation = NULL;
	fileBasicInformation = (FILE_BASIC_INFO*)malloc(size);


	if (!(GetFileInformationByHandleEx(hFileOpen, FileBasicInfo, fileBasicInformation, size))) {

		printf("GetFileInformationByHandleEx()1 is failed %d.\n", GetLastError());
		return false;
	}

	if (!SetFileInformationByHandle(hFileCreate, FileBasicInfo, fileBasicInformation, size)) {

		printf("SetFileInformationByHandle() is failed %d .\n", GetLastError());
		return false;
	}

	free(fileBasicInformation);
}

void fileBackupOperation(char sourcePath[], char destiPath[], char* fileName) {



	HANDLE HsourceFile, HdestiFile;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA ffd;
	TCHAR szDir[MAX_PATH];
	char dfPath[MAX_PATH];
	char sfPath[MAX_PATH];
	LARGE_INTEGER filesize;
	char   ReadBuffer[BUFFERSIZE] = { 0 };
	DWORD NumberOfBytesRead = 0;
	LPDWORD lpNumberOfBytesRead = &NumberOfBytesRead;



	// for conversion of char to wchar_t*

	size_t newSourSize = strlen(sourcePath) + 1;
	wchar_t* wcSource = new wchar_t[newSourSize];
	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, wcSource, newSourSize, sourcePath, _TRUNCATE);

	StringCchCopy(szDir, MAX_PATH, wcSource);
	StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

	hFind = FindFirstFile(szDir, &ffd);
	if (hFind == INVALID_HANDLE_VALUE) {

		printf("FindFirst file is fail %d\n", GetLastError());
		return;
	}


	do
	{
		_bstr_t b(ffd.cFileName);
		char* c = b;

		if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

			if (strcmp(c, fileName) == 0) {

				sourceBackupFileName(c, sourcePath, sfPath, false);
				destiBackupFileName(sfPath, destiPath, dfPath);
				//printf("destiPath==in do while=========================%s\n", dfPath);
				//printf("sourcePath==in do while=========================%s\n", sfPath);

				// read file on Source//
				size_t newSourSize = strlen(sfPath) + 1;
				wchar_t* wcSource = new wchar_t[newSourSize];
				size_t convertedSourChars = 0;
				mbstowcs_s(&convertedSourChars, wcSource, newSourSize, sfPath, _TRUNCATE);

				HsourceFile = CreateFile(wcSource, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

				if (HsourceFile == INVALID_HANDLE_VALUE) {
					printf("Error in Source file creation %d \n", GetLastError());
					return;
				}


				// write file on detination//
				size_t newDestiSize = strlen(dfPath) + 1;
				wchar_t* wcDestin = new wchar_t[newDestiSize];
				size_t convertedDestiChars = 0;
				mbstowcs_s(&convertedDestiChars, wcDestin, newDestiSize, dfPath, _TRUNCATE);

				HdestiFile = CreateFile(wcDestin, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

				if (HdestiFile == INVALID_HANDLE_VALUE) {
					printf("Error in Destination file creation %d \n", GetLastError());
					return;
				}

				while ((ReadFile(HsourceFile, ReadBuffer, sizeof(ReadBuffer), lpNumberOfBytesRead, NULL))) {

					if (NumberOfBytesRead > 0)
						(WriteFile(HdestiFile, ReadBuffer, NumberOfBytesRead, NULL, NULL));
					else
						break;

					RtlZeroMemory(ReadBuffer, BUFFERSIZE);
				}

				MetadatModify(HsourceFile, HdestiFile);

				CloseHandle(HdestiFile);  // detination file handle close
				CloseHandle(HsourceFile); // source file handle close
				delete(wcDestin);
				delete(wcSource);

			}
		}

	} while (FindNextFile(hFind, &ffd) != 0);

	printf("program is Done\n");

	//CloseHandle(hFind);
}


/////////////////////////////////////code end///////////////////////////////////////////////////////////////////////////
void fileRestoreOperation(char sourcePath[], char destiPath[]) {

	TCHAR szDir[MAX_PATH];
	char sfPath[MAX_PATH];
	char dFile[MAX_PATH];
	HANDLE hFind = INVALID_HANDLE_VALUE;
	HANDLE HdestiReFile, HsourceReFile;
	WIN32_FIND_DATA fdd;
	char   ReadBuffer[BUFFERSIZE] = { 0 };
	DWORD NumberOfBytesRead = 0;
	LPDWORD lpNumberOfBytesRead = &NumberOfBytesRead;

	size_t newSourSize = strlen(sourcePath) + 1;
	wchar_t* wcSource = new wchar_t[newSourSize];
	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, wcSource, newSourSize, sourcePath, _TRUNCATE);

	StringCchCopy(szDir, MAX_PATH, wcSource);
	StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

	hFind = FindFirstFile(szDir, &fdd);
	if (hFind == INVALID_HANDLE_VALUE) {

		printf("FindFirst file is fail %d\n", GetLastError());
		return;
	}

	do
	{
		_bstr_t b(fdd.cFileName);
		char* cdeti = b;

		if (!(fdd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

			sourceBackupFileName(destiPath, sourcePath, dFile, true);

			if (strcmp(cdeti, dFile) == 0) {

				sourceBackupFileName(cdeti, sourcePath, sfPath, false);

				size_t newSourSize = strlen(sfPath) + 1;
				wchar_t* wcSource = new wchar_t[newSourSize];
				size_t convertedSourChars = 0;
				mbstowcs_s(&convertedSourChars, wcSource, newSourSize, sfPath, _TRUNCATE);

				HsourceReFile = CreateFile(wcSource, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

				if (HsourceReFile == INVALID_HANDLE_VALUE) {
					printf("Error in Source file creation %d \n", GetLastError());
					return;
				}

				//destination file path conversion

				size_t newDestiSize = strlen(destiPath) + 1;
				wchar_t* wcDestin = new wchar_t[newDestiSize];
				size_t convertedDestiChars = 0;
				mbstowcs_s(&convertedDestiChars, wcDestin, newDestiSize, destiPath, _TRUNCATE);


				//code for open source file and write it into destination file 

				HdestiReFile = CreateFile(wcDestin, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

				if (HdestiReFile == INVALID_HANDLE_VALUE) {
					printf("Error in Destination file creation %d \n", GetLastError());
					return;
				}

				while ((ReadFile(HsourceReFile, ReadBuffer, sizeof(ReadBuffer), lpNumberOfBytesRead, NULL))) {

					if (NumberOfBytesRead > 0)
						(WriteFile(HdestiReFile, ReadBuffer, NumberOfBytesRead, NULL, NULL));
					else
						break;

					RtlZeroMemory(ReadBuffer, BUFFERSIZE);
				}

				MetadatModify(HsourceReFile, HdestiReFile);

				CloseHandle(HdestiReFile);  // detination file handle close
				CloseHandle(HsourceReFile); // source file handle close
				delete(wcDestin);
				delete(wcSource);

			}
		}

	} while (FindNextFile(hFind, &fdd) != 0);
}



void ReleaseInterface(IUnknown* unkn)
{

	if (unkn != NULL)
		unkn->Release();

}

int restore()
{
	vector<char*> bpath;
	BSTR XML, XML2;
	IVssAsync* complete = NULL;
	IVssAsync* async = NULL;
	IVssAsync* pback = NULL;
	IVssBackupComponents* bcomp = NULL;
	IVssExamineWriterMetadata* wdata = NULL;
	IVssWriterComponentsExt* wcomp;
	IVssAsync* ppAsync = NULL;
	VSS_ID  classId = { 0x66841cd4,0x6ded,0x4f4b,{0x8f,0x17,0xfd,0x23,0xf8,0xdd,0xc3,0xde} };
	IVssComponent* vcomp = NULL;
	OUT BSTR path, name;
	VSS_COMPONENT_TYPE type;

	FILE* fs;
	fs = fopen("rmd.txt", "r+");
	FILE* fs1;
	fs1 = fopen("wmd.txt", "r+");
	string request;
	int i;
	while ((i = fgetc(fs)) != EOF)
	{
		request += (char)i;
	}
	XML = _com_util::ConvertStringToBSTR(request.c_str());
	string request1;
	while ((i = fgetc(fs1)) != EOF)
	{
		request1 += (char)i;
	}
	XML2 = _com_util::ConvertStringToBSTR(request1.c_str());
	HRESULT result = CreateVssBackupComponents(&bcomp);
	if (result != S_OK)
	{
		printf("CreateVssBackupComponent error:0x%08lx\n", result);
		ReleaseInterface(bcomp);
		exit(2);
	}
	result = bcomp->InitializeForRestore(XML);
	if (result != S_OK)
	{
		printf("InitializeForRestore error:0x%08lx\n", result);
		ReleaseInterface(bcomp);
		exit(2);
	}
	result = CreateVssExamineWriterMetadata(XML2, &wdata);
	if (result != S_OK)
	{
		printf("CreateVssExamineWriterMetadata error:0x%08lx\n", result);
		ReleaseInterface(bcomp);
		exit(2);
	}
	result = bcomp->GatherWriterMetadata(&async);
	if (result != S_OK)
	{
		printf("GatherWriterMetadata error:0x%08lx\n", result);
		ReleaseInterface(bcomp);
		exit(2);
	}
	printf("Starting Identity Event for restore\n");
	async->Wait();

	cout << "Hello" << endl;
	result = bcomp->GetWriterComponents(0, &wcomp);
	if (result != S_OK)
	{
		printf("GetWriterComponent error:0x%08lx\n", result);
		ReleaseInterface(bcomp);
		exit(2);
	}
	UINT count;
	result = wcomp->GetComponentCount(&count);
	cout << count << endl;
	for (int i = 0; i < count; i++)
	{
		result = wcomp->GetComponent(i, &vcomp);
		if (result != S_OK)
		{
			printf("GetComponent error:0x%08lx\n", result);
			ReleaseInterface(bcomp);
			exit(2);
		}
		vcomp->GetLogicalPath(&path);
		vcomp->GetComponentType(&type);
		vcomp->GetComponentName(&name);
		wprintf_s(L"Path: %s\n", path);
		char* lpszText2 = _com_util::ConvertBSTRToString(path);
		bpath.push_back(lpszText2);
		result = bcomp->SetSelectedForRestore(classId, type, path, name, true);
		if (result != S_OK)
		{
			printf("SetSelectedForRestore error:0x%08lx\n", result);
			ReleaseInterface(bcomp);
			exit(2);
		}
	}
	result = bcomp->PreRestore(&ppAsync);
	if (result != S_OK)
	{
		printf("PreRestor error:0x%08lx\n", result);
		ReleaseInterface(bcomp);
		exit(2);
	}
	cout << "Pre restore in progress" << endl;
	ppAsync->Wait();
	cout << "Pre restore Completed" << endl;

	for (int i = 0; i < bpath.size(); i++)
	{
		//cout<<bpath.at(i)<<endl;//Call Restore Here.
		fileRestoreOperation(backUpFile, bpath.at(i));
	}

	cout << "Restore Complete" << endl;
	result = bcomp->PostRestore(&ppAsync);
	if (result != S_OK)
	{
		printf("PostRestore error:0x%08lx\n", result);
		ReleaseInterface(bcomp);
		exit(2);
	}
	ppAsync->Wait();
	cout << "Post Restore task complete" << endl;

	/*

	result = scomp->GetDirectedTarget(iDirectedTarget, pbstrSourcePath, pbstrSourceFileName, pbstrSourceRangeList, pbstrDestinationPath, pbstrDestinationFilename,
		pbstrDestinationRangeList);
	if (result != S_OK)
	{
		printf("GetDirectedTarget error:0x%08lx\n", result);
		ReleaseInterface(bcomp);
		exit(2);
	}

	*/
}






#define HILONG(ll) (ll >> 32 & LONG_MAX)
#define LOLONG(ll) ((long)(ll))



int main()
{
	if (CoInitialize(NULL) != S_OK)
	{
		cout << "CoInitialize failed!\n";
		return 1;
	}

	VSS_ID classId = { 0x66841cd4,0x6ded,0x4f4b,{0x8f,0x17,0xfd,0x23,0xf8,0xdd,0xc3,0xde} };
	IVssAsync* complete = NULL;
	VSS_ID setid;
	VSS_ID sid[64];
	VSS_ID instId;
	IVssAsync* async = NULL;
	IVssAsync* pback = NULL;
	IVssBackupComponents* bcomp = NULL;
	IVssExamineWriterMetadata* wdata = NULL;
	IVssWMComponent* wcomp = NULL;
	UINT noWriter, inc, exc, comp;
	VSS_USAGE_TYPE usage;
	VSS_SOURCE_TYPE source;
	BSTR path, name;
	VSS_ID rclassId;
	PVSSCOMPONENTINFO cinfo;
	string c[50];
	IVssWMFiledesc* file = NULL;
	IVssAsync* sCopy = NULL;
	vector<char*> ppath;
	vector<char*> pfile;
	BSTR XML2;
	BSTR fileSpec;
	string rstr;
	int n = 0;

	FILE* fs;
	fs = fopen("rmd.txt", "w+");
	FILE* fs1;
	fs1 = fopen("wmd.txt", "w+");
	HRESULT result = CreateVssBackupComponents(&bcomp);
	if (result != S_OK)
	{
		printf("CreateVssBackupComponent error:0x%08lx\n", result);
		ReleaseInterface(bcomp);
		exit(2);
	}
	result = bcomp->InitializeForBackup();
	if (result != S_OK)
	{
		printf("InitializeForBackup error:0x%08lx\n", result);
		ReleaseInterface(bcomp);
		exit(2);
	}
	result = bcomp->SetContext(VSS_CTX_BACKUP | VSS_CTX_APP_ROLLBACK);
	if (result != S_OK)
	{
		printf("SetContext error:0x%08lx\n", result);
		ReleaseInterface(bcomp);
		exit(2);
	}
	result = bcomp->EnableWriterClasses(&classId, 1);
	if (result != S_OK)
	{
		printf("EnableWriterClasses error:0x%08lx\n", result);
		ReleaseInterface(bcomp);
		exit(2);
	}
	result = bcomp->GatherWriterMetadata(&async);
	if (result != S_OK)
	{
		printf("GatherWriterMetadata error:0x%08lx\n", result);
		ReleaseInterface(bcomp);
		exit(2);
	}
	cout << "Gathering Writer Metadata..." << endl;
	async->Wait();
	result = bcomp->GetWriterMetadata(0, &classId, &wdata);
	if (result != S_OK)
	{
		printf("GetWriterMetadata error:0x%08lx\n", result);
		ReleaseInterface(bcomp);
		exit(2);
	}
	result = wdata->SaveAsXML(&XML2);
	if (result != S_OK)
	{
		printf("SaveAsXML error:0x%08lx\n", result);
		ReleaseInterface(bcomp);
		exit(2);
	}
	//wprintf_s(L"XML: \n%s\n\n", XML2);
	char* text1 = _com_util::ConvertBSTRToString(XML2);
	fputs(text1, fs1);
	fclose(fs1);
	result = wdata->GetFileCounts(&inc, &exc, &comp);
	printf("Included Files: %d\n", inc);
	printf("Excluded Files: %d\n", exc);
	printf("Component Files: %d\n\n", comp);
	result = bcomp->StartSnapshotSet(&setid);
	if (result != S_OK)
	{
		printf("StartSnapshotSet error:0x%08lx\n", result);
		ReleaseInterface(bcomp);
		exit(2);
	}
	for (int i = 0; i < comp; i++)
	{
		result = wdata->GetComponent(i, &wcomp);
		if (result != S_OK)
		{
			printf("GetComponent error:0x%08lx\n", result);
			ReleaseInterface(bcomp);
			exit(2);
		}
		result = wdata->GetIdentity(&instId, &rclassId, &name, &usage, &source);
		if (result != S_OK)
		{
			printf("GetIdentity error:0x%08lx\n", result);
			ReleaseInterface(bcomp);
			exit(2);
		}
		result = wcomp->GetComponentInfo(&cinfo);
		if (result != S_OK)
		{
			printf("GetComponentInfo error:0x%08lx\n", result);
			ReleaseInterface(bcomp);
			exit(2);
		}
		if (cinfo->cFileCount != 0)
		{
			wprintf_s(L"Name: %s\n", cinfo->bstrComponentName);
			for (int j = 0; j < cinfo->cFileCount; j++)
			{
				result = wcomp->GetFile(j, &file);
				file->GetPath(&path);
				char* lpszText2 = _com_util::ConvertBSTRToString(path);
				file->GetFilespec(&fileSpec);
				char* lpszText3 = _com_util::ConvertBSTRToString(fileSpec);
				wprintf_s(L"File Path: %s\n", path);
				string str(lpszText2);
				string str1(lpszText3);
				int len = str.length();
				if (str[len - 1] != '\\')
				{
					str += '\\';

				}
				rstr = str;
				str += str1;
				path = _com_util::ConvertStringToBSTR(str.c_str());
				lpszText2 = _com_util::ConvertBSTRToString(path);
				result = bcomp->AddComponent(instId, rclassId, cinfo->type, path, cinfo->bstrComponentName);
				if ((result == S_OK) && (str1 != ""))
				{
					path = _com_util::ConvertStringToBSTR(rstr.c_str());
					char* a = _com_util::ConvertBSTRToString(path);
					ppath.push_back(a);
					path = _com_util::ConvertStringToBSTR(str1.c_str());
					char* b = _com_util::ConvertBSTRToString(path);
					pfile.push_back(b);
					if (str.find(".vhdx") != std::string::npos)
					{
						activate_rct(str);
					}
				}
				int first = str.find_first_of("\\");
				string sub = str.substr(0, first + 1);
				for (int l = 0; l <= j;)
				{
					if (c[l] == "")
					{
						c[l] = sub;
						TCHAR volume[MAX_PATH];
						_tcscpy_s(volume, CA2T(sub.c_str()));
						result = bcomp->AddToSnapshotSet(volume, GUID_NULL, &sid[n]);
						if (result != S_OK)
						{
							printf("AddToSnapshotSet error:0x%08lx\n", result);
							ReleaseInterface(bcomp);
							exit(2);
						}
						n++;
					}
					else if (c[l] != sub)
					{
						l++;
						continue;
					}
					else
					{
						break;
					}

				}
			}
			printf("\n");
		}
	}

	BSTR XML;
	result = bcomp->SaveAsXML(&XML);
	//wprintf_s(L"XML: \n%s\n\n", XML);
	char* text = _com_util::ConvertBSTRToString(XML);
	fputs(text, fs);
	fclose(fs);
	result = bcomp->SetBackupState(true, false, VSS_BT_INCREMENTAL);
	if (result != S_OK)
	{
		printf("SetBackupState error:0x%08lx\n", result);
		ReleaseInterface(bcomp);
		exit(2);
	}
	result = bcomp->PrepareForBackup(&pback);
	if (result != S_OK)
	{
		printf("PrepareForBackup error:0x%08lx\n", result);
		ReleaseInterface(bcomp);
		exit(2);
	}
	printf("Preparing for backup..\n");
	pback->Wait();
	result = pback->QueryStatus(&result, NULL);
	if (!SUCCEEDED(result))
	{
		cout << "Query Status error" << endl;
		ReleaseInterface(bcomp);
		exit(2);
	}
	result = bcomp->DoSnapshotSet(&sCopy);
	if (result != S_OK)
	{
		printf("DoSnapshotSet error:0x%08lx\n", result);
		ReleaseInterface(bcomp);
		exit(2);
	}
	printf("Taking Snap Shot...\n");
	sCopy->Wait();
	printf("Get the snapshot device object from the properties...\n");
	PVSS_SNAPSHOT_PROP snapshotProp = (VSS_SNAPSHOT_PROP*)malloc((n + 1) * sizeof(VSS_SNAPSHOT_PROP));
	cout << "Number of copies:" << snapshotProp->m_lSnapshotsCount << endl;
	for (int i = 0; i < n; i++)
	{
		result = bcomp->GetSnapshotProperties(sid[i], &snapshotProp[i]);
		if (result != S_OK)
		{
			printf("GetSnapshotProperties error:0x%08lx\n", result);
			ReleaseInterface(bcomp);
			exit(2);
		}
		cout << "Snapshot of volume:" << endl;
		wcout << snapshotProp[i].m_pwszOriginalVolumeName << endl;
		WCHAR drive[MAX_PATH];
		PDWORD dlen = NULL;
		GetVolumePathNamesForVolumeNameW(snapshotProp[i].m_pwszOriginalVolumeName, drive, MAX_PATH, dlen);
		wcout << drive << endl;
		wstring ws(drive);
		string drivel(ws.begin(), ws.end());

		char brrFile[] = "C:\\repVSSVol";
		wchar_t* snapVol = snapshotProp[i].m_pwszSnapshotDeviceObject;
		_bstr_t b(snapVol);
		char* cSnapshot = b;
		for (int j = 0; (j < ppath.size()) && (j < pfile.size()); j++)
		{
			string comp(ppath.at(j));
			string comp1 = comp.substr(0, 3);
			if (drivel == comp1)
			{
				char dfPath[MAX_PATH] = { '\0' };

				BackupFileName(ppath.at(j), cSnapshot, dfPath, 1);
				wchar_t wtext[MAX_PATH];
				mbstowcs(wtext, dfPath, strlen(dfPath) + 1);//Plus null
				LPWSTR ptr = wtext;

				if (CreateSymbolicLink(L"C:\\repVSSVol", ptr, SYMBOLIC_LINK_FLAG_DIRECTORY) == 0)
					printf("error in create symbolic link function \n", GetLastError());
				fileBackupOperation(brrFile, backUpFile, pfile.at(j));
				if (RemoveDirectoryA("C:\\repVSSVol")) {
					printf("file is deleted successfully \n");
				}
			}
		}
	}

	result = bcomp->BackupComplete(&complete);
	if (result != S_OK)
	{
		printf("BackUpComplete error: 0x%08lx\n", result);
		ReleaseInterface(bcomp);
		exit(2);
	}
	complete->Wait();
	ReleaseInterface(bcomp);
	cout << "Calling Restore" << endl;
	restore();
	return 0;
}