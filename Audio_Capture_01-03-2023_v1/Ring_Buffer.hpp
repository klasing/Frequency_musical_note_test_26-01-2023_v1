#pragma once
//*****************************************************************************
//*                     include
//*****************************************************************************
#include "framework.h"
#include "Audio_Capture_01-03-2023_v1.h"
//*****************************************************************************
//*                     createRingBuffer
//*****************************************************************************
VOID*
createRingBuffer(UINT bufferSize
    , VOID** secondaryView
)
{
    WCHAR wszBuffer[BUFFER_MAX] = { '\0' };
    BOOL result;
    HANDLE section = nullptr;
    SYSTEM_INFO sysInfo;
    VOID* ringBuffer = nullptr;
    VOID* placeholder1 = nullptr;
    VOID* placeholder2 = nullptr;
    VOID* view1 = nullptr;
    VOID* view2 = nullptr;

    GetSystemInfo(&sysInfo);

    if ((bufferSize % sysInfo.dwAllocationGranularity) != 0) {
        return nullptr;
    }

    // reserve a placeholder region where the buffer will be mapped
    placeholder1 = (PCHAR)VirtualAlloc2(nullptr
        , nullptr
        , 2 * bufferSize
        , MEM_RESERVE | MEM_RESERVE_PLACEHOLDER
        , PAGE_NOACCESS
        , nullptr
        , 0
    );

    if (placeholder1 == nullptr) {
        swprintf_s(wszBuffer
            , (size_t)BUFFER_MAX
            , L"VirtualAlloc2 failed, error %#x\n"
            , GetLastError()
        );
        OutputDebugString(wszBuffer);
        goto Exit;
    }

    // split the placeholder region into two regions of equal size
    result = VirtualFree(
        placeholder1,
        bufferSize,
        MEM_RELEASE | MEM_PRESERVE_PLACEHOLDER
    );

    if (result == FALSE) {
        swprintf_s(wszBuffer
            , (size_t)BUFFER_MAX
            , L"VirtualFreeEx failed, error %#x\n"
            , GetLastError()
        );
        OutputDebugString(wszBuffer);
        goto Exit;
    }

    placeholder2 = (void*)((ULONG_PTR)placeholder1 + bufferSize);

    // create a pagefile-backed section for the buffer
    section = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        nullptr,
        PAGE_READWRITE,
        0,
        bufferSize, nullptr
    );

    if (section == nullptr) {
        swprintf_s(wszBuffer
            , (size_t)BUFFER_MAX
            , L"CreateFileMapping failed, error %#x\n"
            , GetLastError()
        );
        OutputDebugString(wszBuffer);
        goto Exit;
    }

    // map the section into the first placeholder region
    view1 = MapViewOfFile3(
        section,
        nullptr,
        placeholder1,
        0,
        bufferSize,
        MEM_REPLACE_PLACEHOLDER,
        PAGE_READWRITE,
        nullptr, 0
    );

    if (view1 == nullptr) {
        swprintf_s(wszBuffer
            , (size_t)BUFFER_MAX
            , L"MapViewOfFile3 failed, error %#x\n"
            , GetLastError()
        );
        OutputDebugString(wszBuffer);
        goto Exit;
    }

    // ownership transferred, don’t free this now
    placeholder1 = nullptr;

    // map the section into the second placeholder region
    view2 = MapViewOfFile3(
        section,
        nullptr,
        placeholder2,
        0,
        bufferSize,
        MEM_REPLACE_PLACEHOLDER,
        PAGE_READWRITE,
        nullptr, 0
    );

    if (view2 == nullptr) {
        swprintf_s(wszBuffer
            , (size_t)BUFFER_MAX
            , L"MapViewOfFile3 failed, error %#x\n"
            , GetLastError()
        );
        OutputDebugString(wszBuffer);
        goto Exit;
    }

    // success, return both mapped views to the caller
    ringBuffer = view1;
    *secondaryView = view2;

    placeholder2 = nullptr;
    view1 = nullptr;
    view2 = nullptr;
Exit:
    if (section != nullptr) CloseHandle(section);
    if (placeholder1 != nullptr) VirtualFree(placeholder1, 0, MEM_RELEASE);
    if (placeholder2 != nullptr) VirtualFree(placeholder2, 0, MEM_RELEASE);
    if (view1 != nullptr) UnmapViewOfFileEx(view1, 0);
    if (view2 != nullptr) UnmapViewOfFileEx(view2, 0);

    return ringBuffer;
}
