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

#include <GL/glew.h>

class GLShader;

class GLDOPPEngine
{
public:

    GLDOPPEngine();
    virtual ~GLDOPPEngine();

    bool                initDOPP(unsigned int uiDesktop = 1, bool bPresent = true);

    bool                setDesktop(unsigned int uiDesktop);

    virtual bool        initEffect();
    virtual void        updateTexture();

    void                processDesktop(bool bBlocking = false);

    GLuint              getPresentTexture()     { return m_uiPresentTexture;    };
    GLuint              getPresentBuffer()      { return m_uiFBO;               };
    GLuint              getDesktopTexture()     { return m_uiDesktopTexture;    };    

    unsigned int        getDesktopWidth()       { return m_uiDesktopWidth;  };
    unsigned int        getDesktopHeight()      { return m_uiDesktopHeight; }; 

protected:

    void                createQuad();

    GLuint              m_uiDesktopTexture;
    GLuint              m_uiPresentTexture;

    GLuint              m_uiVertexBuffer;
    GLuint              m_uiVertexArray;

    GLShader*           m_pShader;
    GLuint              m_uiBaseMap;

    unsigned int        m_uiDesktopWidth;
    unsigned int        m_uiDesktopHeight; 

private:

    bool                setupDOPPExtension();

    GLuint              m_uiFBO;
   
    bool                m_bStartPostProcessing;
    bool                m_bDoPresent;
};