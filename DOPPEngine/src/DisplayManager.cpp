//
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include <string>
#include <tchar.h>

#include "ADL/adl_sdk.h"
#include <windows.h>

#include "DisplayManager.h"

typedef int(*ADL_MAIN_CONTROL_CREATE)            (ADL_MAIN_MALLOC_CALLBACK, int);
typedef int(*ADL_MAIN_CONTROL_DESTROY)           ();
typedef int(*ADL_ADAPTER_NUMBEROFADAPTERS_GET)   (int*);
typedef int(*ADL_ADAPTER_ACTIVE_GET)             (int, int*);
typedef int(*ADL_ADAPTER_ADAPTERINFO_GET)        (LPAdapterInfo, int);
typedef int(*ADL_ADAPTER_ACTIVE_GET)             (int, int*);
typedef int(*ADL_DISPLAY_DISPLAYINFO_GET)        (int, int*, ADLDisplayInfo**, int);
typedef int(*ADL_DISPLAY_PROPERTY_GET)           (int, int, ADLDisplayProperty*);
typedef int(*ADL_DISPLAY_MODES_GET)              (int, int, int*, ADLMode**);
typedef int(*ADL_DISPLAY_POSITION_GET)           (int, int, int*, int*, int*, int*, int*, int*, int*, int*, int*, int*);
typedef int(*ADL_DISPLAY_SIZE_GET)               (int, int, int*, int*, int*, int*, int*, int*, int*, int*, int*, int*);


typedef struct
{
    HMODULE                             hDLL;
    ADL_MAIN_CONTROL_CREATE             ADL_Main_Control_Create;
    ADL_MAIN_CONTROL_DESTROY            ADL_Main_Control_Destroy;
    ADL_ADAPTER_NUMBEROFADAPTERS_GET    ADL_Adapter_NumberOfAdapters_Get;
    ADL_ADAPTER_ACTIVE_GET              ADL_Adapter_Active_Get;
    ADL_ADAPTER_ADAPTERINFO_GET         ADL_Adapter_AdapterInfo_Get;
    ADL_DISPLAY_DISPLAYINFO_GET         ADL_Display_DisplayInfo_Get;
    ADL_DISPLAY_PROPERTY_GET		    ADL_Display_Property_Get;
    ADL_DISPLAY_MODES_GET               ADL_Display_Modes_Get;
    ADL_DISPLAY_POSITION_GET		    ADL_Display_Position_Get;
    ADL_DISPLAY_SIZE_GET                ADL_Display_Size_Get;
} ADLFunctions;


static ADLFunctions g_AdlCalls = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };


typedef struct DisplayData
{
    unsigned int        uiGPUId;
    unsigned int        uiDisplayId;
    unsigned int        uiDisplayLogicalId;
    int                 nOriginX;
    int                 nOriginY;
    unsigned int        uiWidth;
    unsigned int        uiHeight;
    std::string         strDisplayname;
} DISPLAY_DATA;


// Memory Allocation function needed for ADL
void* __stdcall ADL_Alloc(int iSize)
{
    void* lpBuffer = malloc(iSize);
    return lpBuffer;
}


// Memory Free function needed for ADL
void __stdcall ADL_Free(void* lpBuffer)
{
    if (nullptr != lpBuffer)
    {
        free(lpBuffer);
        lpBuffer = nullptr;
    }
}


using namespace std;


DisplayManager::DisplayManager()
    : m_uiNumGPU(0)
{
    m_Displays.clear();
}


DisplayManager::~DisplayManager()
{
    deleteDisplays();
}


unsigned int DisplayManager::enumDisplays()
{
    int				nNumDisplays = 0;
    int				nNumAdapters = 0;
    int             nCurrentBusNumber = 0;
    LPAdapterInfo   pAdapterInfo = nullptr;
    unsigned int    uiCurrentGPUId = 0;
    unsigned int    uiCurrentDisplayId = 0;

    // load all required ADL functions
    if (!setupADL())
    {
        return 0;
    }

    if (m_Displays.size() > 0)
    {
        deleteDisplays();
    }

    // Determine how many adapters and displays are in the system
    g_AdlCalls.ADL_Adapter_NumberOfAdapters_Get(&nNumAdapters);

    if (nNumAdapters > 0)
    {
        pAdapterInfo = static_cast<LPAdapterInfo>(ADL_Alloc(sizeof(AdapterInfo) * nNumAdapters));
        memset(pAdapterInfo, '\0', sizeof(AdapterInfo) * nNumAdapters);
    }

    g_AdlCalls.ADL_Adapter_AdapterInfo_Get(pAdapterInfo, sizeof(AdapterInfo) * nNumAdapters);

    // Loop through all adapters 
    for (int i = 0; i < nNumAdapters; ++i)
    {
        int nAdapterIdx;
        int nAdapterStatus;

        nAdapterIdx = pAdapterInfo[i].iAdapterIndex;

        g_AdlCalls.ADL_Adapter_Active_Get(nAdapterIdx, &nAdapterStatus);

        if (nAdapterStatus)
        {
            LPADLDisplayInfo pDisplayInfo = nullptr;

            g_AdlCalls.ADL_Display_DisplayInfo_Get(nAdapterIdx, &nNumDisplays, &pDisplayInfo, 0);

            for (int j = 0; j < nNumDisplays; ++j)
            {
                // check if display is connected and mapped
                if (pDisplayInfo[j].iDisplayInfoValue & ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED)
                {
                    // check if display is mapped on adapter
                    if ((pDisplayInfo[j].iDisplayInfoValue & ADL_DISPLAY_DISPLAYINFO_DISPLAYMAPPED) && (pDisplayInfo[j].displayID.iDisplayLogicalAdapterIndex == nAdapterIdx))
                    {
                        if (nCurrentBusNumber == 0)
                        {
                            // Found first GPU in the system
                            ++m_uiNumGPU;
                            nCurrentBusNumber = pAdapterInfo[nAdapterIdx].iBusNumber;
                        }
                        else if (nCurrentBusNumber != pAdapterInfo[nAdapterIdx].iBusNumber)
                        {
                            // found new GPU
                            ++m_uiNumGPU;
                            ++uiCurrentGPUId;
                            nCurrentBusNumber = pAdapterInfo[nAdapterIdx].iBusNumber;
                        }

                        // Found first mapped display
                        DisplayData* pDsp = new DisplayData;

                        pDsp->uiGPUId = uiCurrentGPUId;
                        pDsp->uiDisplayId = uiCurrentDisplayId;
                        pDsp->uiDisplayLogicalId = pDisplayInfo[j].displayID.iDisplayLogicalIndex;
                        pDsp->strDisplayname = string(pAdapterInfo[i].strDisplayName);
                        pDsp->nOriginX = 0;
                        pDsp->nOriginY = 0;
                        pDsp->uiWidth = 0;
                        pDsp->uiHeight = 0;

                        DEVMODEA DevMode;
                        memset(&DevMode, 0, sizeof(DEVMODEA));

                        EnumDisplaySettingsA(pDsp->strDisplayname.c_str(), ENUM_CURRENT_SETTINGS, &DevMode);

                        pDsp->nOriginX = DevMode.dmPosition.x;
                        pDsp->nOriginY = DevMode.dmPosition.y;
                        pDsp->uiWidth = DevMode.dmPelsWidth;
                        pDsp->uiHeight = DevMode.dmPelsHeight;

                        m_Displays.push_back(pDsp);

                        ++uiCurrentDisplayId;
                    }
                }
            }

            ADL_Free(pDisplayInfo);
        }
    }

    if (pAdapterInfo)
    {
        ADL_Free(pAdapterInfo);
    }

    return static_cast<unsigned int>(m_Displays.size());
}


void DisplayManager::deleteDisplays()
{
    for (const auto& pDisplay : m_Displays)
    {
        if (pDisplay)
        {
            delete pDisplay;
        }
    }

    m_Displays.clear();

    m_uiNumGPU = 0;
}


unsigned int DisplayManager::getNumGPUs()
{
    if (m_Displays.size() > 0)
    {
        return m_uiNumGPU;
    }

    return 0;
}


unsigned int DisplayManager::getNumDisplays()
{
    return static_cast<unsigned int>(m_Displays.size());
}


unsigned int DisplayManager::getNumDisplaysOnGPU(unsigned int uiGPU)
{
    unsigned int uiNumDsp = 0;

    for (const auto& pDisplay : m_Displays)
    {
        if (pDisplay->uiGPUId == uiGPU)
        {
            ++uiNumDsp;
        }
    }

    return uiNumDsp;
}


unsigned int DisplayManager::getDisplayOnGPU(unsigned int uiGPU, unsigned int n)
{
    unsigned int uiCurrentDisplayOnGpu = 0;

    for (const auto& pDisplay : m_Displays)
    {
        if (pDisplay->uiGPUId == uiGPU)
        {
            if (uiCurrentDisplayOnGpu == n)
            {
                return pDisplay->uiDisplayId;
            }

            ++uiCurrentDisplayOnGpu;
        }
    }

    return 0;
}

const char* DisplayManager::getDisplayName(unsigned int uiDisplay)
{
    if (uiDisplay < m_Displays.size())
    {
        return m_Displays[uiDisplay]->strDisplayname.c_str();
    }

    return nullptr;
}


unsigned int DisplayManager::getGpuId(unsigned int uiDisplay)
{
    if (uiDisplay < m_Displays.size())
    {
        return m_Displays[uiDisplay]->uiGPUId;
    }

    return 0;
}


bool DisplayManager::getOrigin(unsigned int uiDisplay, int& nOriginX, int& nOriginY)
{
    nOriginX = 0;
    nOriginY = 0;

    if (uiDisplay < m_Displays.size())
    {
        nOriginX = m_Displays[uiDisplay]->nOriginX;
        nOriginY = m_Displays[uiDisplay]->nOriginY;

        return true;
    }

    return false;
}


bool DisplayManager::getSize(unsigned int uiDisplay, unsigned int& uiWidth, unsigned int& uiHeight)
{
    uiWidth = 0;
    uiHeight = 0;

    if (uiDisplay < m_Displays.size())
    {
        uiWidth = m_Displays[uiDisplay]->uiWidth;
        uiHeight = m_Displays[uiDisplay]->uiHeight;

        return true;
    }

    return false;
}


bool DisplayManager::setupADL()
{
    // check if ADL was already loaded
    if (g_AdlCalls.hDLL)
    {
        return true;
    }

    g_AdlCalls.hDLL = LoadLibrary(L"atiadlxx.dll");

    if (g_AdlCalls.hDLL == nullptr)
    {
        g_AdlCalls.hDLL = LoadLibrary(L"atiadlxy.dll");
    }

    if (!g_AdlCalls.hDLL)
    {
        return false;
    }

    // Get proc address of needed ADL functions
    g_AdlCalls.ADL_Main_Control_Create = (ADL_MAIN_CONTROL_CREATE)GetProcAddress(g_AdlCalls.hDLL, "ADL_Main_Control_Create");
    if (!g_AdlCalls.ADL_Main_Control_Create)
    {
        return false;
    }

    g_AdlCalls.ADL_Main_Control_Destroy = (ADL_MAIN_CONTROL_DESTROY)GetProcAddress(g_AdlCalls.hDLL, "ADL_Main_Control_Destroy");
    if (!g_AdlCalls.ADL_Main_Control_Destroy)
    {
        return false;
    }

    g_AdlCalls.ADL_Adapter_NumberOfAdapters_Get = (ADL_ADAPTER_NUMBEROFADAPTERS_GET)GetProcAddress(g_AdlCalls.hDLL, "ADL_Adapter_NumberOfAdapters_Get");
    if (!g_AdlCalls.ADL_Adapter_NumberOfAdapters_Get)
    {
        return false;
    }

    g_AdlCalls.ADL_Adapter_AdapterInfo_Get = (ADL_ADAPTER_ADAPTERINFO_GET)GetProcAddress(g_AdlCalls.hDLL, "ADL_Adapter_AdapterInfo_Get");
    if (!g_AdlCalls.ADL_Adapter_AdapterInfo_Get)
    {
        return false;
    }

    g_AdlCalls.ADL_Display_DisplayInfo_Get = (ADL_DISPLAY_DISPLAYINFO_GET)GetProcAddress(g_AdlCalls.hDLL, "ADL_Display_DisplayInfo_Get");
    if (!g_AdlCalls.ADL_Display_DisplayInfo_Get)
    {
        return false;
    }

    g_AdlCalls.ADL_Adapter_Active_Get = (ADL_ADAPTER_ACTIVE_GET)GetProcAddress(g_AdlCalls.hDLL, "ADL_Adapter_Active_Get");
    if (!g_AdlCalls.ADL_Adapter_Active_Get)
    {
        return false;
    }

    g_AdlCalls.ADL_Display_Position_Get = (ADL_DISPLAY_POSITION_GET)GetProcAddress(g_AdlCalls.hDLL, "ADL_Display_Position_Get");
    if (!g_AdlCalls.ADL_Display_Position_Get)
    {
        return false;
    }

    g_AdlCalls.ADL_Display_Size_Get = (ADL_DISPLAY_POSITION_GET)GetProcAddress(g_AdlCalls.hDLL, "ADL_Display_Size_Get");
    if (!g_AdlCalls.ADL_Display_Size_Get)
    {
        return false;
    }

    g_AdlCalls.ADL_Display_Modes_Get = (ADL_DISPLAY_MODES_GET)GetProcAddress(g_AdlCalls.hDLL, "ADL_Display_Modes_Get");
    if (!g_AdlCalls.ADL_Display_Modes_Get)
    {
        return false;
    }

    g_AdlCalls.ADL_Display_Property_Get = (ADL_DISPLAY_PROPERTY_GET)GetProcAddress(g_AdlCalls.hDLL, "ADL_Display_Property_Get");
    if (!g_AdlCalls.ADL_Display_Property_Get)
    {
        return false;
    }

    // Init ADL
    if (g_AdlCalls.ADL_Main_Control_Create(ADL_Alloc, 0) != ADL_OK)
    {
        return false;
    }

    return true;
}