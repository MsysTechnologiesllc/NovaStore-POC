
#include<vss.h>
#include<iostream>
#include<vswriter.h>
#include<vsbackup.h>
#include <comutil.h>
#include<tchar.h>
#include <atlstr.h>
#pragma warning(disable:4996)
#include <vector>
using namespace std;



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

	/*for (int i = 0;i < bpath.size();i++)
	{
		cout<<bpath.at(i)<<endl;//Call Restore Here.
	}*/

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
	vector<char*> bpath;
	BSTR XML2;
	BSTR fileSpec;

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
	result = bcomp->SetContext(VSS_CTX_BACKUP | VSS_CTX_CLIENT_ACCESSIBLE_WRITERS | VSS_CTX_APP_ROLLBACK);
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
				str += str1;
				path = _com_util::ConvertStringToBSTR(str.c_str());
				lpszText2 = _com_util::ConvertBSTRToString(path);
				result = bcomp->AddComponent(instId, rclassId, cinfo->type, path, cinfo->bstrComponentName);
				if (result == S_OK)
				{
					bpath.push_back(lpszText2);
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
						result = bcomp->AddToSnapshotSet(volume, GUID_NULL, &setid);
						if (result != S_OK)
						{
							printf("AddToSnapshotSet error:0x%08lx\n", result);
							ReleaseInterface(bcomp);
							exit(2);
						}
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
	VSS_SNAPSHOT_PROP snapshotProp = { 0 };
	result = bcomp->GetSnapshotProperties(setid, &snapshotProp);
	if (result != S_OK)
	{
		printf("GetSnapshotProperties error:0x%08lx\n", result);
		ReleaseInterface(bcomp);
		exit(2);
	}

	//display out snapsot properties
	//printf("Snapshot Id :%.8x-%.4x-%.4x-%.2x%.2x-%.2x%.2x%.2x%.2x%.2x%.2x", GUID_PRINTF_ARG(snapshotProp.m_SnapshotId));
	//_tprintf (_T(" Snapshot Set Id ")  WSTR_GUID_FMT _T("\n"), GUID_PRINTF_ARG(snapshotProp.m_SnapshotSetId));
	//_tprintf (_T(" Provider Id ")  WSTR_GUID_FMT _T("\n"), GUID_PRINTF_ARG(snapshotProp.m_ProviderId));													//
	//_tprintf (_T(" OriginalVolumeName : %ls\n"),snapshotProp.m_pwszOriginalVolumeName);
	//if (snapshotProp.m_pwszExposedName != NULL)	_tprintf (_T(" ExposedName : %ls\n"), snapshotProp.m_pwszExposedName);
	//if (snapshotProp.m_pwszExposedPath != NULL)	_tprintf (_T(" ExposedPath : %ls\n"), snapshotProp.m_pwszExposedPath);
	//if (snapshotProp.m_pwszSnapshotDeviceObject != NULL) _tprintf (_T(" DeviceObject : %ls\n"), snapshotProp.m_pwszSnapshotDeviceObject);													

	/**/
	SYSTEMTIME stUTC, stLocal;
	FILETIME ftCreate;
	// Convert the last-write time to local time.
	ftCreate.dwHighDateTime = HILONG(snapshotProp.m_tsCreationTimestamp);
	ftCreate.dwLowDateTime = LOLONG(snapshotProp.m_tsCreationTimestamp);
	FileTimeToSystemTime(&ftCreate, &stUTC);
	SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

	printf("Created : %02d/%02d/%d  %02d:%02d \n", stLocal.wMonth, stLocal.wDay, stLocal.wYear, stLocal.wHour, stLocal.wMinute);
	printf("\n");
	cout << "Before Complete" << endl;


	/*for (int i = 0;i < bpath.size();i++)
	{
		printf("%s\n", bpath.at(i)); //Call backup here.
	}*/

	result = bcomp->BackupComplete(&complete);
	if (result != S_OK)
	{
		printf("BackUpComplete error: 0x%08lx\n", result);
		ReleaseInterface(bcomp);
		exit(2);
	}
	complete->Wait();
	long dsnap;
	VSS_ID idsnap;
	result = bcomp->DeleteSnapshots(snapshotProp.m_SnapshotId, VSS_OBJECT_SNAPSHOT, false, &dsnap, &idsnap);
	if (result != S_OK)
	{
		printf("DeleteSnapshots error: 0x%08lx\n", result);
		ReleaseInterface(bcomp);
		exit(2);
	}
	printf("No of snapshot deleted: %ld\n", dsnap);

	ReleaseInterface(bcomp);
	cout << "Calling Restore" << endl;
	restore();
	return 0;
}