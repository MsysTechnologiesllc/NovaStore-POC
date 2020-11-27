#include <iostream>
#include <string>
#include <cstdlib>
#define WINVER _WIN32_WINNT_WIN10
#include <windows.h>
#include <winioctl.h>
#include <virtdisk.h>
#include <initguid.h>
#include<string.h>
using namespace std;
#pragma comment(lib, "virtdisk.lib")

#import "C:\Users\Administrator\Source\Repos\NovaStore-POC\RCT\RCT\bin\Debug\RCT.tlb" raw_interfaces_only
using namespace RCT;
DEFINE_GUID(VIRTUAL_STORAGE_TYPE_VENDOR_MICROSOFT, 0xec984aec, 0xa0f9, 0x47e9, 0x90, 0x1f, 0x71, 0x41, 0x5a, 0x66, 0x34, 0x5b);
DEFINE_GUID(VIRTUAL_STORAGE_TYPE_VENDOR_UNKNOWN, 0x00000000, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

int main()
{
    HRESULT hr = CoInitialize(NULL);
    CommPtr point(__uuidof(Class1));
    BSTR vmName = SysAllocString(L"Ubuntu2"); 
    BSTR rct;
    point->GetReference(vmName, &rct);
    wcout << "RCT:" << rct << endl;
    HANDLE vhdHandle;
    _VIRTUAL_STORAGE_TYPE storageType;
    storageType.DeviceId = VIRTUAL_STORAGE_TYPE_DEVICE_UNKNOWN;
    storageType.VendorId = { 0xec984aec, 0xa0f9, 0x47e9, {0x90, 0x1f, 0x71, 0x41, 0x5a, 0x66, 0x34, 0x5b} };


    wchar_t path[] = L"C:\\Users\\Public\\Documents\\Hyper-V\\Virtual hard disks\\Ubuntu2.vhdx";
    VIRTUAL_DISK_ACCESS_MASK mask = VIRTUAL_DISK_ACCESS_ALL;

    PGET_VIRTUAL_DISK_INFO diskInfo;
    ULONG diskInfoSize = sizeof(GET_VIRTUAL_DISK_INFO);
    std::wcout << "size of diskinfo structure " << diskInfoSize << std::endl;
    diskInfo = (PGET_VIRTUAL_DISK_INFO)malloc(diskInfoSize);
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
    //diskInfo->Version = SET_VIRTUAL_DISK_INFO_CHANGE_TRACKING_STATE;
    //diskInfo->ChangeTrackingEnabled = true;
    //res = SetVirtualDiskInformation(vhdHandle, diskInfo);

    /*res = GetVirtualDiskInformation(vhdHandle, &diskInfoSize, diskInfo, NULL);
    if (res != ERROR_SUCCESS)
    {
        std::cout << "Failed to GET_VIRTUAL_DISK_INFO_SIZE, ret=" << res << std::endl;
    }
    if (diskInfo->ChangeTrackingState.Enabled == true)
    {
        std::cout << "YES" << std::endl;
    }
    else
    {
        std::cout << "NO" << std::endl;
    }*/
    //std::cout << diskInfo->ChangeTrackingState.Enabled << std::endl<<std::endl;
    diskInfo->Version = GET_VIRTUAL_DISK_INFO_SIZE;

    res = GetVirtualDiskInformation(vhdHandle, &diskInfoSize, diskInfo, NULL);
    if (res != ERROR_SUCCESS)
    {
        std::cout << "Failed to GET_VIRTUAL_DISK_INFO_SIZE, ret=" << res << std::endl;
    }
    long long physicalSize = diskInfo->Size.PhysicalSize;
    long virtualSize = diskInfo->Size.VirtualSize;
    long sectorSize = diskInfo->Size.SectorSize;
    long blockSize = diskInfo->Size.BlockSize;
    std::wcout << "physicalSize :" << physicalSize << std::endl;
    std::wcout << "virtualSize :" << virtualSize << std::endl;
    std::wcout << "sectorSize :" << sectorSize << std::endl;
    std::wcout << "blockSize :" << blockSize << std::endl;

    diskInfo->Version = GET_VIRTUAL_DISK_INFO_CHANGE_TRACKING_STATE;

    res = GetVirtualDiskInformation(vhdHandle, &diskInfoSize, diskInfo, NULL);
    if (res != ERROR_SUCCESS)
    {
        std::cout << "Failed to GET_VIRTUAL_DISK_INFO_CHANGE_TRACKING_STATE, ret=" << res << std::endl;
    }
    //std::cout << "\nrct id:" << diskInfo->ChangeTrackingState.MostRecentId << std::endl;

    std::cout << "\nQuerying for changed disk areas...\n" << std::endl;


    wchar_t rctId[] = L"rctX:e59e6991:208a:44d9:ae6a:2f14351d792f:00000000"; //RCT id used here is not from c# called method.
    std::wcout << L"rct:" << rctId << std::endl;
    ULONG64   byteOffset = 0L;
    ULONG64   byteLength = physicalSize;
    QUERY_CHANGES_VIRTUAL_DISK_RANGE changedAreas[1000];
    ULONG     rangeCount = 1000;
    ULONG64   processedLength = 0L;
    res = 0;
    res = QueryChangesVirtualDisk(vhdHandle, rctId, byteOffset, byteLength,
        QUERY_CHANGES_VIRTUAL_DISK_FLAG_NONE,
        changedAreas, &rangeCount, &processedLength);
    cout << "Range Count" << rangeCount << endl;
    if (res != ERROR_SUCCESS)
    {
        std::cout << "Failed to get changed areas, ret=0x\n" << res;
        if (vhdHandle != NULL)
        {
            CloseHandle(vhdHandle);
            std::cout << "closing handle!" << std::endl;
        }
        return 0;
    }

    std::cout << "Total changed areas:" << rangeCount << std::endl;
    std::cout << "Total processed length:" << processedLength << std::endl;

    for (int i = 0; i < rangeCount; i++)
    {
        wprintf(L"ByteOffset : %lu   ByteLength : %lu\n", changedAreas[i].ByteOffset, changedAreas[i].ByteLength);
    }

    if (res != ERROR_SUCCESS)
    {
        std::cout << L"QueryChangesVirtualDisk fail\n" << std::endl;
        goto Cleanup;
    }

Cleanup:

    if (res == ERROR_SUCCESS)
    {
        std::wcout << L"success\n";
    }
    else
    {
        std::wcout << L"error = %u\n", res;
    }

    if (vhdHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(vhdHandle);
    }

    if (diskInfo != NULL)
    {
        free(diskInfo);
    }

    

    return res;
}