/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2011 Steven Lovegrove
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <pangolin/gl/viewport.h>
#include <pangolin/gl/glinclude.h>
#include <pangolin/image/image_io.h>

#if defined(HAVE_EIGEN) && !defined(__CUDACC__) //prevent including Eigen in cuda files
#define USE_EIGEN
#endif

#ifdef USE_EIGEN
#include <Eigen/Core>
#endif

#include <cstdlib>
#include <iostream>
#include <math.h>

namespace pangolin
{

////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////

class PANGOLIN_EXPORT GlTexture
{
public:
    //! internal_format normally one of GL_RGBA8, GL_LUMINANCE8, GL_INTENSITY16
    GlTexture(GLint width, GLint height, GLint internal_format = GL_RGBA8, bool sampling_linear = true, int border = 0, GLenum glformat = GL_RGBA, GLenum gltype = GL_UNSIGNED_BYTE, GLvoid* data = NULL  );

    // Construct this texture from a CPU image
    GlTexture(const TypedImage& img, bool sampling_linear=true);
    
    //! Move Constructor / asignment
    GlTexture(GlTexture&& tex);
    GlTexture& operator=(GlTexture&& tex);
    
    //! Default constructor represents 'no texture'
    GlTexture();
    virtual ~GlTexture();

    bool IsValid() const;

    //! Delete OpenGL resources and fall back to representing 'no texture'
    void Delete();
    
    //! Reinitialise teture width / height / format
    virtual void Reinitialise(GLsizei width, GLsizei height, GLint internal_format = GL_RGBA8, bool sampling_linear = true, int border = 0, GLenum glformat = GL_RGBA, GLenum gltype = GL_UNSIGNED_BYTE, GLvoid* data = NULL );
    
    void Bind() const;
    void Unbind() const;
    
    //! data_layout normally one of GL_LUMINANCE, GL_RGB, ...
    //! data_type normally one of GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_FLOAT
    void Upload(const void* image, GLenum data_format = GL_LUMINANCE, GLenum data_type = GL_FLOAT);
    
    //! Upload data to texture, overwriting a sub-region of it.
    //! data ptr contains packed data_w x data_h of pixel data.
    void Upload(const void* data,
        GLsizei tex_x_offset, GLsizei tex_y_offset,
        GLsizei data_w, GLsizei data_h,
        GLenum data_format, GLenum data_type
    );

    void Load(const TypedImage& image, bool sampling_linear = true);

    void LoadFromFile(const std::string& filename, bool sampling_linear = true);

    void Download(void* image, GLenum data_layout = GL_LUMINANCE, GLenum data_type = GL_FLOAT) const;

    void Download(TypedImage& image) const;

    void CopyFrom(const GlTexture& tex);

    void Save(const std::string& filename, bool top_line_first = true);

    void SetLinear();
    void SetNearestNeighbour();
    
    void RenderToViewport(const bool flip) const;
    void RenderToViewport() const;
    void RenderToViewport(Viewport tex_vp, bool flipx=false, bool flipy=false) const;
    void RenderToViewportFlipY() const;
    void RenderToViewportFlipXFlipY() const;
    
    GLint internal_format;
    GLuint tid;
    GLint width;
    GLint height;
    
private:
    // Private copy constructor
    GlTexture(const GlTexture&) {}
};

struct PANGOLIN_EXPORT GlRenderBuffer
{
    GlRenderBuffer();
    GlRenderBuffer(GLint width, GLint height, GLint internal_format = GL_DEPTH_COMPONENT24);

    void Reinitialise(GLint width, GLint height, GLint internal_format = GL_DEPTH_COMPONENT24);

    //! Move Constructor
    GlRenderBuffer(GlRenderBuffer&& tex);

    ~GlRenderBuffer();
    
    GLint width;
    GLint height;
    GLuint rbid;

private:
    // Private copy constructor
    GlRenderBuffer(const GlRenderBuffer&) {}
};

struct PANGOLIN_EXPORT GlFramebuffer
{
    GlFramebuffer();
    ~GlFramebuffer();
    
    GlFramebuffer(GlTexture& colour);
    GlFramebuffer(GlTexture& colour, GlRenderBuffer& depth);
    GlFramebuffer(GlTexture& colour0, GlTexture& colour1, GlRenderBuffer& depth);
    GlFramebuffer(GlTexture& colour0, GlTexture& colour1, GlTexture& colour2, GlRenderBuffer& depth);
    GlFramebuffer(GlTexture& colour0, GlTexture& colour1, GlTexture& colour2, GlTexture& colour3, GlRenderBuffer& depth);

    void Bind() const;
    void Unbind() const;

    void Reinitialise();
    
    // Attach Colour texture to frame buffer
    // Return attachment texture is bound to (e.g. GL_COLOR_ATTACHMENT0_EXT)
    GLenum AttachColour(GlTexture& tex);
    
    // Attach Depth render buffer to frame buffer
    void AttachDepth(GlRenderBuffer& rb);
    
    GLuint fbid;
    unsigned attachments;
};

enum GlBufferType
{
    GlUndefined = 0,
    GlArrayBuffer = GL_ARRAY_BUFFER,                    // VBO's, CBO's, NBO's
    GlElementArrayBuffer = GL_ELEMENT_ARRAY_BUFFER,     // IBO's
#ifndef HAVE_GLES
    GlPixelPackBuffer = GL_PIXEL_PACK_BUFFER,           // PBO's
    GlPixelUnpackBuffer = GL_PIXEL_UNPACK_BUFFER,
//    GlShaderStorageBuffer = GL_SHADER_STORAGE_BUFFER
#endif
};

// This encapsulates a GL Buffer object.
struct PANGOLIN_EXPORT GlBufferData
{
    //! Default constructor represents 'no buffer'
    GlBufferData();

    GlBufferData(GlBufferType buffer_type, GLsizeiptr size_bytes, GLenum gluse = GL_DYNAMIC_DRAW, const void *data = 0 );

    template<typename T>
    GlBufferData(GlBufferType buffer_type, const std::vector<T>& data, GLenum gluse = GL_STATIC_DRAW);

    virtual ~GlBufferData();

    void Free();

    //! Move Constructor
    GlBufferData(GlBufferData&& tex);
    GlBufferData& operator=(GlBufferData&& tex);

    bool IsValid() const;

    GLsizeiptr SizeBytes() const;
    
    void Reinitialise(GlBufferType buffer_type, GLsizeiptr size_bytes, GLenum gluse = GL_DYNAMIC_DRAW, const void *data = 0 );
    
    void Bind() const;
    void Unbind() const;
    void Upload(const GLvoid* data, GLsizeiptr size_bytes, GLintptr offset = 0);
    void Download(GLvoid* ptr, GLsizeiptr size_bytes, GLintptr offset = 0) const;
    

    template<typename T>
    void Upload(const std::vector<T>& data, GLintptr offset = 0);

#ifdef USE_EIGEN
    template<typename Derived>
    void Upload(const Eigen::DenseBase<Derived>& data, GLintptr offset = 0);
#endif

    GLuint bo;
    GlBufferType buffer_type;
    GLenum gluse;
    GLsizeiptr size_bytes;

private:
    GlBufferData(const GlBufferData&) {}
};

// This encapsulates a GL Buffer object, also storing information about its contents.
// You should try to use GlBufferData instead.
struct PANGOLIN_EXPORT GlBuffer : public GlBufferData
{
    //! Default constructor represents 'no buffer'
    GlBuffer();
    GlBuffer(GlBufferType buffer_type, GLuint num_elements, GLenum datatype, GLuint count_per_element, GLenum gluse = GL_DYNAMIC_DRAW );
    GlBuffer(const GlBuffer&) = delete;
    
    //! Move Constructor
    GlBuffer(GlBuffer&& tex);
    GlBuffer& operator=(GlBuffer&& tex);
    
    void Reinitialise(GlBufferType buffer_type, GLuint num_elements, GLenum datatype, GLuint count_per_element, GLenum gluse, const void* data = nullptr );
    void Reinitialise(GlBuffer const& other );
    void Resize(GLuint num_elements);

#ifdef USE_EIGEN
    template<typename Scalar, int R, int C>
    GlBuffer(GlBufferType buffer_type, const std::vector<Eigen::Matrix<Scalar, R,C>>& data, GLenum gluse = GL_STATIC_DRAW);
#endif
            
    GLenum datatype;
    GLuint num_elements;
    GLuint count_per_element;
};

class PANGOLIN_EXPORT GlSizeableBuffer
        : public pangolin::GlBuffer
{
public:
    GlSizeableBuffer(pangolin::GlBufferType buffer_type, GLuint initial_num_elements, GLenum datatype, GLuint count_per_element, GLenum gluse = GL_DYNAMIC_DRAW );
    
    void Clear();
    
#ifdef USE_EIGEN
    template<typename Derived>
    void Add(const Eigen::DenseBase<Derived>& vec);
    
    template<typename Derived>
    void Update(const Eigen::DenseBase<Derived>& vec, size_t position = 0);
#endif
    
    size_t start() const;
    
    size_t size() const;
    
protected:  
    void CheckResize(size_t num_verts);
    
    size_t NextSize(size_t min_size) const;
    
    size_t  m_num_verts;    
};

size_t GlFormatChannels(GLenum data_layout);

size_t GlDataTypeBytes(GLenum type);

TypedImage ReadFramebuffer(const Viewport& v, const std::string& pixel_format = "RGBA32");

}

// Include implementation
#include <pangolin/gl/gl.hpp>
