///////////////////////////////////////////////////////////////////////////////////////
//
//    Envy Game Engine
//    https://github.com/PatrickTorgerson/Envy
//
//    Copyright (c) 2021 Patrick Torgerson
//
//    Permission is hereby granted, free of charge, to any person obtaining a copy
//    of this software and associated documentation files (the "Software"), to deal
//    in the Software without restriction, including without limitation the rights
//    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//    copies of the Software, and to permit persons to whom the Software is
//    furnished to do so, subject to the following conditions:
//
//    The above copyright notice and this permission notice shall be included in all
//    copies or substantial portions of the Software.
//
//    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//    SOFTWARE.
//
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "common.hpp"

#include "directx.hpp"

#include <vector>
#include <initializer_list>

namespace Envy
{

    template <typename T>
    class gfxbuffer
    {
    public:

        using element_t = T;

    private:

        ComPtr<ID3D12Resource> upload;
        ComPtr<ID3D12Resource> buffer;
        std::vector<element_t> verteces;

    public:

        gfxbuffer() {}

        gfxbuffer(std::initializer_list<element_t> verts)
            : verteces {verts.begin(), verts.end()}
        {

            // -- create buffers

            auto upload_props { CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) };
            auto upload_desc  { CD3DX12_RESOURCE_DESC::Buffer(verteces.size() * sizeof(element_t)) };

            upload = graphics::create_resource(upload_props, upload_desc, D3D12_RESOURCE_STATE_GENERIC_READ);

            auto upload_props { CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT) };
            auto upload_desc  { CD3DX12_RESOURCE_DESC::Buffer(verteces.size() * sizeof(element_t)) };

            buffer = graphics::create_resource(upload_props, upload_desc, D3D12_RESOURCE_STATE_COPY_DEST);

            // -- upload data

            update();
        }

        // push
        // pop
        // []
        // clear
        // set
        // reserve
        // get_verteces

        void update()
        {
            graphics::upload(upload, verteces);
            graphics::copy(upload, buffer);
        }

        ID3D12Resource* operator->()
        { return buffer.Get(); }

        ID3D12Resource* get()
        { return buffer.Get(); }

        ID3D12Resource** addressof()
        { return buffer.GetAddressOf(); }

        ComPtr<ID3D12Resource> comptr()
        { return buffer; }
    };

}