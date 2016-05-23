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

#include "GLDOPPColorInvert.h"
#include "GLShader.h"


GLDOPPColorInvert::GLDOPPColorInvert()
    : GLDOPPEngine()
    , m_pProgram(nullptr)
    , m_uiBasmapLoc(0)
{}


GLDOPPColorInvert::~GLDOPPColorInvert()
{
    if (m_pProgram)
    {
        delete m_pProgram;
    }
}


bool GLDOPPColorInvert::initEffect()
{
    if (m_pProgram)
    {
        delete m_pProgram;
    }

    m_pProgram = new GLShader;

    if ((!m_pProgram->createVertexShaderFromFile("Shaders/base.vp")) || (!m_pProgram->createFragmentShaderFromFile("Shaders/colorInv.fp")))
    {
        return false;
    }

    if (!m_pProgram->buildProgram())
    {
        return false;
    }

    m_uiBasmapLoc = glGetUniformLocation(m_pProgram->getProgram(), "baseMap");

    createQuad();

    return true;
}


void GLDOPPColorInvert::updateTexture()
{
    glDisable(GL_DEPTH_TEST);

    m_pProgram->bind();

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_uiDesktopTexture);

    glUniform1i(m_uiBasmapLoc, 1);

    glBindVertexArray(m_uiVertexArray);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);

    m_pProgram->unbind();
}