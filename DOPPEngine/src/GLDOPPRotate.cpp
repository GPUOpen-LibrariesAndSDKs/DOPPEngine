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

#define _USE_MATH_DEFINES
#include <math.h>

#include "GLDOPPRotate.h"
#include "GLShader.h"


GLDOPPRotate::GLDOPPRotate()
    : m_pProgram(nullptr)
    , m_uiBasmapLoc(0)
{
    // Init model view
    memset(m_pModelViewMat, 0, 16 * sizeof(float));

    m_pModelViewMat[0]  = 1.0f;      // [0][0]
    m_pModelViewMat[5]  = 1.0f;      // [1][1]
    m_pModelViewMat[10] = 1.0f;      // [2][2]
    m_pModelViewMat[15] = 1.0f;      // [3][3]

    // Init ortho projection matrix as identity
    memset(m_pProjectionMat, 0, 16 * sizeof(float));

    m_pProjectionMat[0]  = 1.0f;
    m_pProjectionMat[5]  = 1.0f;
    m_pProjectionMat[10] = 1.0f;
    m_pProjectionMat[15] = 1.0f;
}


GLDOPPRotate::~GLDOPPRotate()
{
    if (m_pProgram)
    {
        delete m_pProgram;
    }

    if (m_uiTransform)
    {
        glDeleteBuffers(1, &m_uiTransform);
    }
}


bool GLDOPPRotate::initEffect()
{
    if (m_pProgram)
    {
        delete m_pProgram;
    }

    m_pProgram = new GLShader;

    if ((!m_pProgram->createVertexShaderFromFile("Shaders/transform.vp")) || (!m_pProgram->createFragmentShaderFromFile("Shaders/base.fp")))
    {
        return false;
    }

    if (!m_pProgram->buildProgram())
    {
        return false;
    }

    m_uiBasmapLoc = glGetUniformLocation(m_pProgram->getProgram(), "baseMap");

    // create UBO to pass tarnsformation matrix
    glGenBuffers(1, &m_uiTransform);

    glBindBuffer(GL_UNIFORM_BUFFER, m_uiTransform);

    glBufferData(GL_UNIFORM_BUFFER, 32 * sizeof(float), m_pModelViewMat, GL_STATIC_DRAW);

    glBufferSubData(GL_UNIFORM_BUFFER, 16 * sizeof(float), 16 * sizeof(float), m_pProjectionMat);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    return true;
}


void GLDOPPRotate::setRotation(float a)
{
    float r = (static_cast<float>(M_PI ) * a) / 180.0f;

    float ar = static_cast<float>(m_uiDesktopWidth) / m_uiDesktopHeight;

    glBindBuffer(GL_UNIFORM_BUFFER, m_uiTransform);

    float* pMat = reinterpret_cast<float*>(glMapBufferRange(GL_UNIFORM_BUFFER, 0, 32 * sizeof(float), GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT));

    if (pMat)
    {
        // Build rotation matrix to rotate quad around z axis and scale y to
        // have a quad that matched the AR of the desktop texture
        pMat[0] = cosf(r);
        pMat[1] = -sinf(r);

        pMat[4] = sinf(r) / ar;
        pMat[5] = cosf(r) / ar;

        // Build ortho projection matrix (glOrtho(-1.0f, 1.0f, -1.0f/ar, 1.0f/ar, -1.0f, 1.0f))
        // Only Orth[1][1] need to be updated. The ortho frustum needs to match the AR of the desktop
        pMat[21] = ar;
    }

    glUnmapBuffer(GL_UNIFORM_BUFFER);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


void GLDOPPRotate::updateTexture()
{
    glDisable(GL_DEPTH_TEST);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_uiTransform);

    m_pProgram->bind();

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_uiDesktopTexture);

    glUniform1i(m_uiBasmapLoc, 1);

    glBindVertexArray(m_uiVertexArray);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);

    m_pProgram->unbind();
}