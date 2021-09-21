#include <graphics.hpp>

#include <window.hpp>
#include <event.hpp>
#include <exception.hpp>
#include <log.hpp>
#include <directx.hpp>

#include <algorithm>
#include <iostream>
#include <locale>
#include <codecvt>

#ifdef assert
#undef assert
#endif

namespace Envy::graphics
{

    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ TU Locals ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

    namespace
    {
        // -- constants

        constexpr u8 frame_count {3u};

        // -- DirectX 12 state

        ComPtr<ID3D12Device2> device;
        ComPtr<IDXGISwapChain4> swapchain;
        ComPtr<ID3D12CommandQueue> commandqueue;
        ComPtr<ID3D12Resource> backbuffers[frame_count];
        ComPtr<ID3D12GraphicsCommandList> commandlist;
        ComPtr<ID3D12CommandAllocator> commandallocators[frame_count];
        ComPtr<ID3D12DescriptorHeap> rtv_descriptorheap;

        u32 rtv_descriptorsize;
        u32 bufferindex;

        // -- syncronization

        ComPtr<ID3D12Fence> fence;
        u64 fence_value {};
        u64 fence_values[frame_count] = {};
        HANDLE fence_event;

        // -- diagnostics

        Envy::logger gfxlog {"Graphics"};

        // -- graphics state

        Envy::vector2<i32> buffersize {640,480};
    }


    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ Helper Forwards ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


    void assert_hresult(HRESULT hr, Envy::string msg, std::source_location loc = std::source_location::current());

    void enable_debug_layer();

    ComPtr<IDXGIAdapter4> find_adapter(bool use_warp);

    ComPtr<ID3D12Device2> create_device(ComPtr<IDXGIAdapter4> adapter);

    ComPtr<ID3D12CommandQueue> create_commandqueue(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type);

    bool check_tearing_support();

    ComPtr<IDXGISwapChain4> create_swapchain(HWND hwnd, ComPtr<ID3D12CommandQueue> commandqueue, u32 width, u32 height, u32 buffer_count);

    ComPtr<ID3D12DescriptorHeap> create_descriptorheap(ComPtr<ID3D12Device2> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t descriptor_count);

    void update_rtvs(ComPtr<ID3D12Device2> device, ComPtr<IDXGISwapChain4> swapchain, ComPtr<ID3D12DescriptorHeap> descriptorheap);

    ComPtr<ID3D12CommandAllocator> create_commandallocator(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type);

    ComPtr<ID3D12GraphicsCommandList create_commandlist(ComPtr<ID3D12Device2> device, ComPtr<ID3D12CommandAllocator> commandallocator, D3D12_COMMAND_LIST_TYPE type);

    ComPtr<ID3D12Fence> create_fence(ComPtr<ID3D12Device2> device);

    HANDLE create_eventhandle();

    u64 signal_fence(ComPtr<ID3D12CommandQueue> commandqueue, ComPtr<ID3D12Fence> fence, uint64_t& fence_value);

    void wait_for_fence(ComPtr<ID3D12Fence> fence, u64 value, HANDLE event, std::chrono::milliseconds duration = std::chrono::milliseconds::max());

    void flush(ComPtr<ID3D12CommandQueue> queue, ComPtr<ID3D12Fence> fence, u64& value, HANDLE event);

    void report_fps();

    void resize(const window::resized& resize);

    void resizing(const window::resizing& resize);


    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ TEMP ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


    using namespace DirectX;

    struct Vertex
    {
        XMFLOAT3 position;
        XMFLOAT4 color;
    };

    CD3DX12_VIEWPORT viewport;
    CD3DX12_RECT scissor;

    ComPtr<ID3D12RootSignature> rootsignature;
    ComPtr<ID3D12PipelineState> pipeline_state;

    ComPtr<ID3D12Resource> vbuff;
    D3D12_VERTEX_BUFFER_VIEW vbuff_view;

    ComPtr<ID3D12Resource> ibuff;
    D3D12_INDEX_BUFFER_VIEW ibuff_view;

    XMMATRIX projectionmat;
    XMMATRIX modelmat;

    f32 x {0.0f};
    f32 y {0.0f};
    f32 xv {2.0f};
    f32 yv {2.0f};

    f32 winw {};
    f32 winh {};


    void hello_triangel()
    {
        viewport = {0.0f, 0.0f, 1904.0f, 998.0f};
        scissor = {0l, 0l, 1904l, 998l};

        projectionmat  =  DirectX::XMMatrixOrthographicOffCenterLH(0.0f, 1904.0f, 998.0f, 0.0f, -1.0f, 1.0f);
        modelmat       =  DirectX::XMMatrixIdentity();

        // Create a root signature.
        {
            D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
            if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
            {
                featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
            }

            D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

            // A single 32-bit constant root parameter that is used by the vertex shader.
            CD3DX12_ROOT_PARAMETER1 rootParameters[1];
            rootParameters[0].InitAsConstants(sizeof(XMMATRIX) / 4, 0);

            CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
            rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

            ComPtr<ID3DBlob> signature;
            ComPtr<ID3DBlob> error;
            assert_hresult(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error), "Could not serialize root signature");
            assert_hresult(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootsignature)), "Could not create root signature");
        }

        // Create the pipeline state, which includes compiling and loading shaders.
        {
            ComPtr<ID3DBlob> vertexShader;
            ComPtr<ID3DBlob> pixelShader;

            UINT compileFlags {};

            if constexpr (Envy::debug)
            {
                // Enable better shader debugging with the graphics debugging tools.
                compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
            }

            ComPtr<ID3DBlob> vserr;
            ComPtr<ID3DBlob> pserr;

            auto vsres = D3DCompileFromFile(L"quads.hlsl", nullptr, nullptr, "vsmain", "vs_5_1", compileFlags, 0, &vertexShader, &vserr);
            auto psres = D3DCompileFromFile(L"quads.hlsl", nullptr, nullptr, "psmain", "ps_5_1", compileFlags, 0, &pixelShader, &pserr);

            if(FAILED(vsres))
            {
                gfxlog.error("Vertex shader compile errors:")
                .note("{}", (vserr) ? (char*) vserr->GetBufferPointer() : "");
            }

            if(FAILED(psres))
            {
                gfxlog.error("Pixel shader compile errors:")
                .note("{}", (pserr) ? (char*) pserr->GetBufferPointer() : "");
            }

            gfxlog.assert(SUCCEEDED(vsres) && SUCCEEDED(psres), "Failed to compile shaders");

            // Define the vertex input layout.
            D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
            };

            // Describe and create the graphics pipeline state object (PSO).
            D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
            psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
            psoDesc.pRootSignature = rootsignature.Get();
            psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
            psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
            psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
            psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
            psoDesc.DepthStencilState.DepthEnable = FALSE;
            psoDesc.DepthStencilState.StencilEnable = FALSE;
            psoDesc.SampleMask = UINT_MAX;
            psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            psoDesc.NumRenderTargets = 1;
            psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
            psoDesc.SampleDesc.Count = 1;
            assert_hresult(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipeline_state)), "could not create pipline state");
        }

        // Create the vertex buffer.
        {
            // Define the geometry for a triangle.
            Vertex triangleVertices[] =
            {
                { {  000.00f, 000.00f, 0.0f }, { 1.0f, 0.2f, 0.2f, 1.0f } },
                { {  200.00f, 000.00f, 0.0f }, { 0.2f, 1.0f, 0.2f, 1.0f } },
                { {  200.00f, 200.00f, 0.0f }, { 0.2f, 0.2f, 1.0f, 1.0f } },
                { {  000.00f, 200.00f, 0.0f }, { 1.0f, 0.2f, 1.0f, 1.0f } }
            };

            const UINT vertexBufferSize = sizeof(triangleVertices);

            // Note: using upload heaps to transfer static data like vert buffers is not
            //       recommended. Every time the GPU needs it, the upload heap will be marshalled
            //       over. Please read up on Default Heap usage. An upload heap is used here for
            //       code simplicity and because there are very few verts to actually transfer.

            auto heap_props {CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD)};
            auto resdesc    {CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize)};

            assert_hresult(device->CreateCommittedResource(
                &heap_props,
                D3D12_HEAP_FLAG_NONE,
                &resdesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&vbuff)),
            "Could not create vertex buffer");

            // Copy the triangle data to the vertex buffer.
            UINT8* pVertexDataBegin;
            CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
            assert_hresult(vbuff->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)), "Could not map vertex buffer");
            memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
            vbuff->Unmap(0, nullptr);

            // Initialize the vertex buffer view.
            vbuff_view.BufferLocation = vbuff->GetGPUVirtualAddress();
            vbuff_view.StrideInBytes = sizeof(Vertex);
            vbuff_view.SizeInBytes = vertexBufferSize;
        }

        // Create index buffer
        {
            // Define the geometry for a triangle.
            u16 indeces[] =
            {
                0,1,2,
                0,2,3
            };

            const UINT indexbuff_size = sizeof(indeces);

            // Note: using upload heaps to transfer static data like vert buffers is not
            // recommended. Every time the GPU needs it, the upload heap will be marshalled
            // over. Please read up on Default Heap usage. An upload heap is used here for
            // code simplicity and because there are very few verts to actually transfer.

            auto heap_props {CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD)};
            auto resdesc {CD3DX12_RESOURCE_DESC::Buffer(indexbuff_size)};

            assert_hresult(device->CreateCommittedResource(
                &heap_props,
                D3D12_HEAP_FLAG_NONE,
                &resdesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&ibuff)),
            "Could not create index buffer");

            // Copy the triangle data to the vertex buffer.
            UINT8* pIndexDataBegin;
            CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
            assert_hresult(ibuff->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin)), "Could not map index buffer");
            memcpy(pIndexDataBegin, indeces, sizeof(indeces));
            ibuff->Unmap(0, nullptr);

            // Initialize the vertex buffer view.
            ibuff_view.BufferLocation = ibuff->GetGPUVirtualAddress();
            ibuff_view.Format = DXGI_FORMAT::DXGI_FORMAT_BC6H_UF16;
            ibuff_view.SizeInBytes = indexbuff_size;
        }

        flush(commandqueue, fence, fence_value, fence_event);
    }


    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ Interface ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


    //**********************************************************************
    void init(const description& gfxdesc)
    {
        // Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
        // Using this awareness context allows the client area of the window
        // to achieve 100% scaling while still allowing non-client window content to
        // be rendered in a DPI sensitive fashion.
        // SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

        ENVY_DEBUG_CALL(enable_debug_layer());

        ComPtr<IDXGIAdapter4> adapter = find_adapter(false);

        device = create_device(adapter);

        // check hardware  tier
        {
            D3D12_FEATURE_DATA_D3D12_OPTIONS featureSupport{};

            device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &featureSupport, sizeof(featureSupport));

            switch (featureSupport.ResourceBindingTier)
            {
            case D3D12_RESOURCE_BINDING_TIER_3:
                gfxlog.info("Hardware Tier 3"); break;

            case D3D12_RESOURCE_BINDING_TIER_2:
                gfxlog.info("Hardware Tier 2"); break;

            case D3D12_RESOURCE_BINDING_TIER_1:
                gfxlog.info("Hardware Tier 1"); break;
            }
        }

        // check memory info
        {
            DXGI_QUERY_VIDEO_MEMORY_INFO localmeminf;
            DXGI_QUERY_VIDEO_MEMORY_INFO nonlocalmeminf;

            if(SUCCEEDED(adapter->QueryVideoMemoryInfo(0u, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &localmeminf)))
            {
                gfxlog.info("VRAM : {:.2f}MB / {:.2f}MB")(localmeminf.CurrentUsage / 1e6, localmeminf.Budget / 1e6);
            }

            if(SUCCEEDED(adapter->QueryVideoMemoryInfo(0u, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &nonlocalmeminf)))
            {
                gfxlog.info(" RAM : {:.2f}MB / {:.2f}MB")(nonlocalmeminf.CurrentUsage / 1e6, nonlocalmeminf.Budget / 1e6);
            }
        }

        commandqueue = create_commandqueue(device, D3D12_COMMAND_LIST_TYPE_DIRECT);

        swapchain = create_swapchain( window::get_hwnd(), commandqueue, 640, 480, frame_count);

        bufferindex = swapchain->GetCurrentBackBufferIndex();

        rtv_descriptorheap = create_descriptorheap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, frame_count);
        rtv_descriptorsize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        update_rtvs(device, swapchain, rtv_descriptorheap);

        for(int i {}; i < frame_count; ++i)
        {
            commandallocators[i] = create_commandallocator(device, D3D12_COMMAND_LIST_TYPE_DIRECT);
        }

        commandlist = create_commandlist(device, commandallocators[bufferindex], D3D12_COMMAND_LIST_TYPE_DIRECT);

        fence = create_fence(device);
        fence_event = create_eventhandle();

        hello_triangel();

        gfxlog.info("Graphics successfully initialized");

        // -- regester event callbacks

        Envy::register_callback<window::resized>(resize);
        Envy::register_callback<window::resizing>(resizing);
    }


    //**********************************************************************
    void shutdown()
    {
        flush(commandqueue, fence, fence_value, fence_event);

        ::CloseHandle(fence_event);
    }


    //**********************************************************************
    void clear()
    {

        // update

        if(x+xv < 0.0f)
        { xv = 2.0f; }

        if(x+xv > winw-200.0f)
        { xv = -2.0f; }

        if(y+yv < 0.0f)
        { yv = 2.0f; }

        if(y+yv > winh-200.0f)
        { yv = -2.0f; }

        x += xv;
        y += yv;

        modelmat = DirectX::XMMatrixTranslation(x,y,0.0f);

        // wait for required resouces to be available
        wait_for_fence(fence, fence_values[bufferindex], fence_event);

        // clear
        auto allocator  { commandallocators[bufferindex] };
        auto backbuffer { backbuffers[bufferindex] };

        allocator->Reset();
        commandlist->Reset(allocator.Get(), pipeline_state.Get());

        commandlist->SetGraphicsRootSignature(rootsignature.Get());
        commandlist->RSSetViewports(1, &viewport);
        commandlist->RSSetScissorRects(1, &scissor);

        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(backbuffer.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        commandlist->ResourceBarrier(1, &barrier);

        f32 clearcolor[] { 0.4f, 0.4f, 0.5f, 1.0f };

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtv { rtv_descriptorheap->GetCPUDescriptorHandleForHeapStart(), (i32) bufferindex, rtv_descriptorsize };

        commandlist->OMSetRenderTargets(1, &rtv, FALSE, nullptr);

        commandlist->ClearRenderTargetView(rtv, clearcolor, 0, nullptr);

        // render

        commandlist->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandlist->IASetVertexBuffers(0, 1, &vbuff_view);
        commandlist->IASetIndexBuffer(&ibuff_view);

        // Update the MVP matrix
        XMMATRIX mvp = DirectX::XMMatrixMultiply(modelmat, projectionmat);
        commandlist->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &mvp, 0);

        commandlist->DrawIndexedInstanced(6,1,0,0,0);
    }


    //**********************************************************************
    void present()
    {
        auto allocator  { commandallocators[bufferindex] };
        auto backbuffer { backbuffers[bufferindex] };

        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(backbuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        commandlist->ResourceBarrier(1, &barrier);

        assert_hresult( commandlist->Close() , "Could not close command list");

        //ID3D12CommandList* const commandsists[] { commandlist.Get() };

        commandqueue->ExecuteCommandLists(1u, (ID3D12CommandList*const*) commandlist.GetAddressOf());

        u32 sync_interval =1; // { g_VSync ? 1 : 0 };
        u32 present_flags =0; // { g_TearingSupported && !g_VSync ? DXGI_PRESENT_ALLOW_TEARING : 0 };

        assert_hresult( swapchain->Present(sync_interval, present_flags) , "Could not present swap chain");

        fence_values[bufferindex] = signal_fence(commandqueue, fence, fence_value);

        bufferindex = swapchain->GetCurrentBackBufferIndex();
    }


    //**********************************************************************
    void resize(const window::resized& resize)
    {
        if(buffersize != resize.size && (buffersize.x < resize.size.x || buffersize.y < resize.size.y))
        {
            buffersize.x = std::max(buffersize.x, resize.size.x);
            buffersize.y = std::max(buffersize.y, resize.size.y);

            // Flush the GPU queue to make sure the swap chain's back buffers
            // are not being referenced by an in-flight command list.
            flush(commandqueue, fence, fence_value, fence_event);

            for(i32 i {}; i < frame_count; ++i)
            {
                // Any references to the back buffers must be released
                // before the swap chain can be resized.
                backbuffers[i].Reset();
                fence_values[i] = fence_values[bufferindex];
            }

            DXGI_SWAP_CHAIN_DESC swapdesc = {};
            assert_hresult( swapchain->GetDesc(&swapdesc) , "Could not get swap chain description");

            assert_hresult( swapchain->ResizeBuffers(frame_count, buffersize.x, buffersize.y, swapdesc.BufferDesc.Format, swapdesc.Flags) , "Could not resize swap chain");

            bufferindex = swapchain->GetCurrentBackBufferIndex();

            update_rtvs(device, swapchain, rtv_descriptorheap);

            gfxlog.info("Graphics buffer resized")
            .note("new size: {}", Envy::to_string(buffersize));

            projectionmat  =  DirectX::XMMatrixOrthographicOffCenterLH(0.0f, (f32) buffersize.x, (f32) buffersize.y, 0.0f, -1.0f, 1.0f);

            viewport = {0.0f, 0.0f, (f32) buffersize.x, (f32) buffersize.y};
        }

        resizing(window::resizing {resize.size});
    }


    //**********************************************************************
    void resizing(const window::resizing& resize)
    {
        // f32 pw { resize.size.x * (buffersize.x \ resize.size.x) };
        // f32 ph {};

        // viewport = {0.0f, 0.0f, (f32) resize.size.x, (f32) resize.size.y};
        scissor = {0l, 0l, (i32) resize.size.x, (i32) resize.size.y};

        winw = (f32) resize.size.x;
        winh = (f32) resize.size.y;

        x = std::min(x,resize.size.x-200.0f);
        y = std::min(y,resize.size.y-200.0f);
    }


    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ Helper Functions ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


    //**********************************************************************
    void assert_hresult(HRESULT hr, Envy::string msg, std::source_location loc)
    {
        gfxlog.assert(SUCCEEDED(hr), "HRESULT Failure: " + msg, loc);
    }


    //**********************************************************************
    void enable_debug_layer()
    {
        // Always enable the debug layer before doing anything DX12 related
        // so all possible errors generated while creating DX12 objects
        // are caught by the debug layer.

        ComPtr<ID3D12Debug> debug_interface;

        assert_hresult( D3D12GetDebugInterface(IID_PPV_ARGS(&debug_interface)) , "Could not enable D3D Debug Layer");

        debug_interface->EnableDebugLayer();
    }


    //**********************************************************************
    ComPtr<IDXGIAdapter4> find_adapter(bool use_warp)
    {
        ComPtr<IDXGIFactory4> dxgi_factory;
        u32 factory_flags {0};

        ENVY_DEBUG_CALL( factory_flags = DXGI_CREATE_FACTORY_DEBUG );

        assert_hresult( CreateDXGIFactory2(factory_flags, IID_PPV_ARGS(&dxgi_factory)) , "Could not create DXGIFactory");

        ComPtr<IDXGIAdapter1> dxgi_adapter1;
        ComPtr<IDXGIAdapter4> dxgi_adapter4;

        if(use_warp)
        {
            assert_hresult( dxgi_factory->EnumWarpAdapter(IID_PPV_ARGS(&dxgi_adapter1)) , "Could not enumerate WARP adapters");
            assert_hresult( dxgi_adapter1.As(&dxgi_adapter4) , "Could not perform required adapter cast");
        }
        else
        {
            SIZE_T max_vram = 0;

            for (u32 i {}; dxgi_factory->EnumAdapters1(i, &dxgi_adapter1) != DXGI_ERROR_NOT_FOUND; ++i)
            {
                DXGI_ADAPTER_DESC1 dxgi_adapter1_desc;
                dxgi_adapter1->GetDesc1(&dxgi_adapter1_desc);

                // Check to see if the adapter can create a D3D12 device without actually
                // creating it. The adapter with the largest dedicated video memory
                // is favored. WARP adapters are ignored

                const bool hardware_adapter  { (dxgi_adapter1_desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 };
                const bool supports_DX12     { SUCCEEDED(D3D12CreateDevice(dxgi_adapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) };
                const bool greater_vram      { dxgi_adapter1_desc.DedicatedVideoMemory > max_vram };

                if(hardware_adapter && supports_DX12 && greater_vram)
                {
                    max_vram = dxgi_adapter1_desc.DedicatedVideoMemory;
                    assert_hresult( dxgi_adapter1.As(&dxgi_adapter4) , "Could not perform required adapter cast");
                }
            }
        }

        DXGI_ADAPTER_DESC3 desc;
        dxgi_adapter4->GetDesc3(&desc);

        char adapter_desc[512];

        WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, adapter_desc, 512, NULL, NULL);

        gfxlog.info("Found suitible graphics adapter '{}'")(adapter_desc);

        return dxgi_adapter4;
    }


    //**********************************************************************
    ComPtr<ID3D12Device2> create_device(ComPtr<IDXGIAdapter4> adapter)
    {
        ComPtr<ID3D12Device2> new_device;

        assert_hresult( D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&new_device)) , "Could not create D3D12 device");

        // -- Enable debug messages in debug mode

        if constexpr (Envy::debug)
        {
            ComPtr<ID3D12InfoQueue> info_queue;

            if(SUCCEEDED(new_device.As(&info_queue)))
            {

                info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
                info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
                info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

                // Suppress whole categories of messages
                // D3D12_MESSAGE_CATEGORY Categories[] = {};

                // Suppress messages based on their severity level
                D3D12_MESSAGE_SEVERITY severities[] =
                {
                    D3D12_MESSAGE_SEVERITY_INFO
                };

                // Suppress individual messages by their ID
                D3D12_MESSAGE_ID ids[] = {
                    D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
                    D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
                    D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
                };

                D3D12_INFO_QUEUE_FILTER filter = {};
                //NewFilter.DenyList.NumCategories = _countof(Categories);
                //NewFilter.DenyList.pCategoryList = Categories;
                filter.DenyList.NumSeverities = _countof(severities);
                filter.DenyList.pSeverityList = severities;
                filter.DenyList.NumIDs = _countof(ids);
                filter.DenyList.pIDList = ids;

                assert_hresult( info_queue->PushStorageFilter(&filter) , "Could not push info queue storage filter");
            }
        }

        return new_device;
    }


    //**********************************************************************
    ComPtr<ID3D12CommandQueue> create_commandqueue(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type)
    {
        ComPtr<ID3D12CommandQueue> new_commandqueue;

        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type      = type;
        desc.Priority  = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        desc.Flags     = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask  = 0;

        assert_hresult( device->CreateCommandQueue(&desc, IID_PPV_ARGS(&new_commandqueue)) , "Could not create command queue");

        return new_commandqueue;
    }


    //**********************************************************************
    bool check_tearing_support()
    {
        BOOL supported {FALSE};

        // Rather than create the DXGI 1.5 factory interface directly, we create the
        // DXGI 1.4 interface and query for the 1.5 interface. This is to enable the
        // graphics debugging tools which will not support the 1.5 factory interface
        // until a future update.

        ComPtr<IDXGIFactory4> factory4;

        if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
        {
            ComPtr<IDXGIFactory5> factory5;

            if (SUCCEEDED(factory4.As(&factory5)))
            {
                if (FAILED(factory5->CheckFeatureSupport(
                    DXGI_FEATURE_PRESENT_ALLOW_TEARING,
                    &supported, sizeof(supported))))
                {
                    supported = FALSE;
                }
            }
        }

        return supported == TRUE;
    }


    //**********************************************************************
    ComPtr<IDXGISwapChain4> create_swapchain(HWND hwnd, ComPtr<ID3D12CommandQueue> commandqueue, u32 width, u32 height, u32 buffer_count)
    {
        ComPtr<IDXGISwapChain4> new_swapchain;
        ComPtr<IDXGIFactory4> dxgi_factory;
        u32 factory_flags {};

        ENVY_DEBUG_CALL( factory_flags = DXGI_CREATE_FACTORY_DEBUG );

        assert_hresult( CreateDXGIFactory2(factory_flags, IID_PPV_ARGS(&dxgi_factory)) , "Could not create DXGIFactory");

        DXGI_SWAP_CHAIN_DESC1 swapchain_desc = {};
        swapchain_desc.Width = width;
        swapchain_desc.Height = height;
        swapchain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapchain_desc.Stereo = FALSE;
        swapchain_desc.SampleDesc = { 1, 0 };
        swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapchain_desc.BufferCount = buffer_count;
        swapchain_desc.Scaling = DXGI_SCALING_NONE;
        swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapchain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        // It is recommended to always allow tearing if tearing support is available.
        swapchain_desc.Flags = check_tearing_support() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

        ComPtr<IDXGISwapChain1> swapchain1;

        assert_hresult(
            dxgi_factory->CreateSwapChainForHwnd(
                commandqueue.Get(),
                hwnd,
                &swapchain_desc,
                nullptr,
                nullptr,
                &swapchain1
            ),
            "Could not create swap chain"
        );

        // Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
        // will be handled manually.
        assert_hresult( dxgi_factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER) , "Could not disable Atl-Enter fullscreen togle");

        assert_hresult( swapchain1.As(&new_swapchain) , "Could not perform required cast");

        return new_swapchain;
    }


    //**********************************************************************
    ComPtr<ID3D12DescriptorHeap> create_descriptorheap(ComPtr<ID3D12Device2> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t descriptor_count)
    {
        ComPtr<ID3D12DescriptorHeap> descriptorheap;

        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = descriptor_count;
        desc.Type = type;

        assert_hresult( device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorheap)) , "Could not create descriptor heap");

        return descriptorheap;
    }


    //**********************************************************************
    void update_rtvs(ComPtr<ID3D12Device2> device, ComPtr<IDXGISwapChain4> swapchain, ComPtr<ID3D12DescriptorHeap> descriptorheap)
    {
        u32 rtv_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        CD3DX12_CPU_DESCRIPTOR_HANDLE hrtv { descriptorheap->GetCPUDescriptorHandleForHeapStart() };

        for (int i = 0; i < frame_count; ++i)
        {
            ComPtr<ID3D12Resource> backbuffer;

            assert_hresult( swapchain->GetBuffer(i, IID_PPV_ARGS(&backbuffer)) , "Could not get back buffer");

            device->CreateRenderTargetView(backbuffer.Get(), nullptr, hrtv);

            backbuffers[i] = backbuffer;

            hrtv.Offset(rtv_size);
        }
    }


    //**********************************************************************
    ComPtr<ID3D12CommandAllocator> create_commandallocator(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type)
    {
        ComPtr<ID3D12CommandAllocator> new_commandallocator;

        assert_hresult( device->CreateCommandAllocator(type, IID_PPV_ARGS(&new_commandallocator)) , "Could not create command allocator");

        return new_commandallocator;
    }


    //**********************************************************************
    ComPtr<ID3D12GraphicsCommandList> create_commandlist(ComPtr<ID3D12Device2> device, ComPtr<ID3D12CommandAllocator> commandallocator, D3D12_COMMAND_LIST_TYPE type)
    {
        ComPtr<ID3D12GraphicsCommandList> new_commandlist;

        assert_hresult( device->CreateCommandList(0, type, commandallocator.Get(), nullptr, IID_PPV_ARGS(&new_commandlist)) , "Could not create commandlist");

        assert_hresult( new_commandlist->Close() , "Could not close commandlist");

        return new_commandlist;
    }


    //**********************************************************************
    ComPtr<ID3D12Fence> create_fence(ComPtr<ID3D12Device2> device)
    {
        ComPtr<ID3D12Fence> new_fence;

        assert_hresult( device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&new_fence)) , "Could not create fence");

        return new_fence;
    }


    //**********************************************************************
    HANDLE create_eventhandle()
    {
        HANDLE new_fence_event;

        new_fence_event = ::CreateEvent(NULL, FALSE, FALSE, NULL);

        gfxlog.assert( new_fence_event != nullptr , "Could not create fence event");

        return new_fence_event;
    }


    //**********************************************************************
    u64 signal_fence(ComPtr<ID3D12CommandQueue> commandqueue, ComPtr<ID3D12Fence> fence, uint64_t& fence_value)
    {
        u64 value = ++fence_value;

        assert_hresult( commandqueue->Signal(fence.Get(), value) , "Could not signal fence");

        return value;
    }


    //**********************************************************************
    void wait_for_fence(ComPtr<ID3D12Fence> fence, u64 value, HANDLE event, std::chrono::milliseconds duration)
    {
        if(fence->GetCompletedValue() < value)
        {
            assert_hresult( fence->SetEventOnCompletion(value, event) , "Could not set fence completion event");
            ::WaitForSingleObject(event, static_cast<DWORD>(duration.count()));
        }
    }


    //**********************************************************************
    void flush(ComPtr<ID3D12CommandQueue> queue, ComPtr<ID3D12Fence> fence, u64& value, HANDLE event)
    {
        u64 wait_val { signal_fence(queue, fence, value) };

        wait_for_fence(fence, wait_val, event);
    }


    //**********************************************************************
    void report_fps()
    {
        static u64 frame_counter {};
        static f64 elapsed {};
        static std::chrono::high_resolution_clock clock;
        static auto t0 {clock.now()};

        ++frame_counter;

        auto t1    { clock.now() };
        std::chrono::duration<double> delta { t1 - t0 };
        t0 = t1;

        elapsed += delta.count();

        if(elapsed > 1.0)
        {
            char buffer[500];
            auto fps = frame_counter / elapsed;

            sprintf_s(buffer, 500, "FPS: %f\n", fps);

            OutputDebugStringA(buffer);

            frame_counter = 0;
            elapsed -= 1.0;
        }
    }

}