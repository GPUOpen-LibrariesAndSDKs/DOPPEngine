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

#include "StdAfx.h"
#include <GL/glew.h>

#include "GLShader.h"
#include "GLDOPPDistort.h"


GLDOPPDistort::GLDOPPDistort()
    : m_pProgram(nullptr)
    , m_uiBasmapLoc(0)
    , m_uiTimerLoc(0)
{
    m_lStartCount.QuadPart = 0;
    QueryPerformanceFrequency(&m_lFreq);
}


GLDOPPDistort::~GLDOPPDistort()
{
    if (m_pProgram)
    {
        delete m_pProgram;
    }
}


bool GLDOPPDistort::initEffect()
{
    if (m_pProgram)
    {
        delete m_pProgram;
    }

    m_pProgram = new GLShader;

    if ((!m_pProgram->createVertexShaderFromFile("Shaders/base.vp")) || (!m_pProgram->createFragmentShaderFromFile("Shaders/distort.fp")))
    {
        return false;
    }

    if (!m_pProgram->buildProgram())
    {
        return false;
    }

    m_uiBasmapLoc = glGetUniformLocation(m_pProgram->getProgram(), "baseMap");
    m_uiTimerLoc = glGetUniformLocation(m_pProgram->getProgram(), "fTime");

    QueryPerformanceCounter((LARGE_INTEGER*)&m_lStartCount);

    return true;
}


void GLDOPPDistort::updateTexture()
{
    LARGE_INTEGER lCounter;
    float fElapsed;

    QueryPerformanceCounter(&lCounter);

    long long lDelta = lCounter.QuadPart - m_lStartCount.QuadPart;

    fElapsed = (static_cast<float>(lDelta) / m_lFreq.QuadPart);

    glDisable(GL_DEPTH_TEST);

    m_pProgram->bind();

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_uiDesktopTexture);

    glUniform1i(m_uiBasmapLoc, 1);
    glUniform1f(m_uiTimerLoc, fElapsed);

    glBindVertexArray(m_uiVertexArray);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);

    m_pProgram->unbind();
}