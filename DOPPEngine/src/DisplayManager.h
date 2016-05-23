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

#pragma once

#include <vector>

struct DisplayData;

class DisplayManager
{
public:

    DisplayManager();
    ~DisplayManager();

    // Enumerates the active desktops. This functions needs to be called before any other method of this class
    // returns the number of active desktops
    unsigned int enumDisplays();

    // returns the number of GPUs in the system
    unsigned int    getNumGPUs();

    unsigned int    getNumDisplays();

    // returns the number of displays mapped on GPU uiGPU
    unsigned int    getNumDisplaysOnGPU(unsigned int uiGPU);

    // returns the DisplayID of n-th Display on GPU uiGPU
    // n=0 will return the first display on GPU uiGPU if available
    // n=1 will return the second display on GPU uiGPU if available ...
    unsigned int    getDisplayOnGPU(unsigned int uiGPU, unsigned int n = 0);

    const char*     getDisplayName(unsigned int uiDisplay);

    unsigned int    getGpuId(unsigned int uiDisplay);

    bool            getOrigin(unsigned int uiDisplay, int& uiOriginX, int& uiOriginY);

    bool            getSize(unsigned int uiDisplay, unsigned int& uiWidth, unsigned int& uiHeight);

private:

    bool            setupADL();
    void            deleteDisplays();

    unsigned int                m_uiNumGPU;

    std::vector<DisplayData*>  m_Displays;
};