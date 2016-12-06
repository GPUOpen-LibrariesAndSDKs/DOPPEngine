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

#include "stdafx.h"

#include <GL/glew.h>
#include <GL/wglew.h>

#include "GLDOPPEngine.h"
#include "GLShader.h"

#define GL_WAIT_FOR_PREVIOUS_VSYNC 0x931C

typedef GLuint(APIENTRY* PFNWGLGETDESKTOPTEXTUREAMD)(void);
typedef void(APIENTRY* PFNWGLENABLEPOSTPROCESSAMD)(bool enable);
typedef GLuint(APIENTRY* WGLGENPRESENTTEXTUREAMD)(void);
typedef GLboolean(APIENTRY* WGLDESKTOPTARGETAMD)(GLuint);
typedef GLuint(APIENTRY* PFNWGLPRESENTTEXTURETOVIDEOAMD)(GLuint presentTexture, const GLuint* attrib_list);

PFNWGLGETDESKTOPTEXTUREAMD      wglGetDesktopTextureAMD;
PFNWGLENABLEPOSTPROCESSAMD      wglEnablePostProcessAMD;
PFNWGLPRESENTTEXTURETOVIDEOAMD  wglPresentTextureToVideoAMD;
WGLDESKTOPTARGETAMD             wglDesktopTargetAMD;
WGLGENPRESENTTEXTUREAMD         wglGenPresentTextureAMD;

#define GET_PROC(xx)                                        \
    {                                                       \
        void **x = reinterpret_cast<void**>(&xx);           \
        *x = static_cast<void*>(wglGetProcAddress(#xx));    \
        if (*x == nullptr) {                                \
            return false;                                   \
        }                                                   \
    }


GLDOPPEngine::GLDOPPEngine()
    : m_uiDesktopWidth(0)
    , m_uiDesktopHeight(0)
    , m_uiDesktopTexture(0)
    , m_uiPresentTexture(0)
    , m_uiFBO(0)
    , m_uiBaseMap(0)
    , m_pShader(nullptr)
    , m_uiVertexBuffer(0)
    , m_uiVertexArray(0)
    , m_bStartPostProcessing(false)
    , m_bDoPresent(false)
{}


GLDOPPEngine::~GLDOPPEngine()
{
    if (m_bDoPresent)
    {
        wglEnablePostProcessAMD(false);
    }

    glFinish();

    if (m_pShader)
    {
        delete m_pShader;
        m_pShader = nullptr;
    }

    if (m_uiDesktopTexture)
    {
        glDeleteTextures(1, &m_uiDesktopTexture);
    }

    if (m_uiFBO)
    {
        glDeleteFramebuffers(1, &m_uiFBO);
    }

    if (m_uiVertexBuffer)
    {
        glDeleteBuffers(1, &m_uiVertexBuffer);
    }

    if (m_uiVertexArray)
    {
        glDeleteVertexArrays(1, &m_uiVertexArray);
    }
}


bool GLDOPPEngine::initDOPP(unsigned int uiDesktop, bool bPresent)
{
    if (!setupDOPPExtension())
    {
        return false;
    }

    // Select Desktop to be processed. ID is the same as seen in CCC
    if (!wglDesktopTargetAMD(uiDesktop))
    {
        return false;
    }

    glActiveTexture(GL_TEXTURE1);

    // Get Desktop Texture. Instead of creating a regular texture we request the desktop texture
    m_uiDesktopTexture = wglGetDesktopTextureAMD();
    glBindTexture(GL_TEXTURE_2D, m_uiDesktopTexture);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    // Get size of the desktop. Usually this is the same values as returned by GetSystemMetrics(SM_CXSCREEN)
    // and GetSystemMetrics(SM_CYSCREEN). In some cases it might differ, e.g. if a rotated desktop is used.
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, reinterpret_cast<GLint*>(&m_uiDesktopWidth));
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, reinterpret_cast<GLint*>(&m_uiDesktopHeight));

    glBindTexture(GL_TEXTURE_2D, 0);

    // Create FBO that is used to generate the present texture. We will render into this FBO 
    // in order to create the present texture
    glGenFramebuffers(1, &m_uiFBO);

    // generate present texture
    m_uiPresentTexture = wglGenPresentTextureAMD();

    glBindTexture(GL_TEXTURE_2D, m_uiPresentTexture);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, m_uiFBO);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uiPresentTexture, 0);

    GLenum FBStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (FBStatus != GL_FRAMEBUFFER_COMPLETE)
    {
        return false;
    }

    m_bStartPostProcessing = bPresent;
    m_bDoPresent = bPresent;

    glGenVertexArrays(1, &m_uiVertexArray);

    return true;
}


bool GLDOPPEngine::initEffect()
{
    if (m_pShader)
    {
        delete m_pShader;
    }

    m_pShader = new GLShader;

    // Load basic shader 
    if (!m_pShader->createVertexShaderFromFile("base.vp"))
    {
        return false;
    }

    if (!m_pShader->createFragmentShaderFromFile("base.fp"))
    {
        return false;
    }

    if (!m_pShader->buildProgram())
    {
        return false;
    }

    m_pShader->bind();

    m_uiBaseMap = glGetUniformLocation(m_pShader->getProgram(), "baseMap");

    return true;
}


bool GLDOPPEngine::setDesktop(unsigned int uIDesktop)
{
    if (wglDesktopTargetAMD(uIDesktop))
    {
        return true;
    }

    return false;
}


void GLDOPPEngine::processDesktop(bool bBlocking)
{
    int pVP[4];

    glGetIntegerv(GL_VIEWPORT, pVP);

    // Bind FBO that has the present texture attached
    glBindFramebuffer(GL_FRAMEBUFFER, m_uiFBO);

    // Set viewport for this FBO
    glViewport(0, 0, m_uiDesktopWidth, m_uiDesktopHeight);

    // Render to FBO
    updateTexture();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (m_bDoPresent)
    {
        const GLuint attrib[] = { (bBlocking ? GL_WAIT_FOR_PREVIOUS_VSYNC : 0), 0 };

        // Set the new desktop texture
        wglPresentTextureToVideoAMD(m_uiPresentTexture, attrib);

        if (m_bStartPostProcessing)
        {
            m_bStartPostProcessing = false;

            wglEnablePostProcessAMD(true);
        }

        glFinish();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // restore original viewport
    glViewport(pVP[0], pVP[1], pVP[2], pVP[3]);
}


void GLDOPPEngine::updateTexture()
{
    m_pShader->bind();

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_uiDesktopTexture);

    glUniform1i(m_uiBaseMap, 1);

    glBindVertexArray(m_uiVertexArray);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    m_pShader->unbind();
}


bool GLDOPPEngine::setupDOPPExtension()
{
    GET_PROC(wglGetDesktopTextureAMD);
    GET_PROC(wglEnablePostProcessAMD);
    GET_PROC(wglPresentTextureToVideoAMD);
    GET_PROC(wglDesktopTargetAMD);
    GET_PROC(wglGenPresentTextureAMD);

    return true;
}