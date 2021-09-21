#include <graphics.hpp>

#include <window.hpp>
#include <event.hpp>
#include <exception.hpp>

#include <algorithm>

#include <LLGL/LLGL.h>
#include <LLGL/Platform/NativeHandle.h>

namespace Envy::graphics
{

    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ Surface ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


    // used by LLGL to interface with our window
    class surface_t final : public LLGL::Surface
    {
    public:

        surface_t();

        virtual bool GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) const override;
        virtual LLGL::Extent2D GetContentSize() const override;
        virtual bool AdaptForVideoMode(LLGL::VideoModeDescriptor& videoModeDesc) override;
        virtual void ResetPixelFormat() override;
        virtual bool ProcessEvents() override;
        virtual std::unique_ptr<LLGL::Display> FindResidentDisplay() const override;
    };


    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ TU locals ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


    namespace
    {
        // -- LLGL state

        std::unique_ptr<LLGL::RenderSystem> llgl;

        LLGL::RenderContext* context;
        LLGL::CommandQueue* queue;
        LLGL::CommandBuffer* commands;

        std::shared_ptr<surface_t> surface;

        // -- Diagnostics

        Envy::logger gfxlog {"Graphics"};
    }


    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ Helper Forwards ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


    void resize(const window::resized& resize);


    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ TEMP ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


    LLGL::PipelineState* pipeline;
    LLGL::Buffer* vertexBuffer;

    void triangle()
    {
        // Vertex data structure
        struct Vertex
        {
            float   position[2];
            uint8_t color[4];
        };

        // Vertex data (3 vertices for our triangle)
        const float s = 0.5f;

        Vertex vertices[] =
        {
            { {  0,  s }, { 255, 0, 0, 255 } }, // 1st vertex: center-top, red
            { {  s, -s }, { 0, 255, 0, 255 } }, // 2nd vertex: right-bottom, green
            { { -s, -s }, { 0, 0, 255, 255 } }, // 3rd vertex: left-bottom, blue
        };

        // Vertex format
        LLGL::VertexFormat vertexFormat;

        // Append 2D float vector for position attribute
        vertexFormat.AppendAttribute({ "position", LLGL::Format::RG32Float });

        // Append 3D unsigned byte vector for color
        vertexFormat.AppendAttribute({ "color",    LLGL::Format::RGBA8UNorm });

        // Update stride in case out vertex structure is not 4-byte aligned
        vertexFormat.SetStride(sizeof(Vertex));

        // Create vertex buffer
        LLGL::BufferDescriptor vertexBufferDesc;
        {
            vertexBufferDesc.size           = sizeof(vertices);                 // Size (in bytes) of the vertex buffer
            vertexBufferDesc.bindFlags      = LLGL::BindFlags::VertexBuffer;    // Enables the buffer to be bound to a vertex buffer slot
            vertexBufferDesc.vertexAttribs  = vertexFormat.attributes;          // Vertex format layout
        }
        vertexBuffer = llgl->CreateBuffer(vertexBufferDesc, vertices);

        // Create shaders
        LLGL::Shader* vertShader = nullptr;
        LLGL::Shader* fragShader = nullptr;

        LLGL::ShaderDescriptor vertShaderDesc, fragShaderDesc;

        vertShaderDesc = { LLGL::ShaderType::Vertex,   "Example.hlsl", "vsmain", "vs_5_0" };
        fragShaderDesc = { LLGL::ShaderType::Fragment, "Example.hlsl", "psmain", "ps_5_0" };

        // Specify vertex attributes for vertex shader
        vertShaderDesc.vertex.inputAttribs = vertexFormat.attributes;

        vertShader = llgl->CreateShader(vertShaderDesc);
        fragShader = llgl->CreateShader(fragShaderDesc);

        for(auto shader : { vertShader, fragShader })
        {
            if (shader != nullptr)
            {
                std::string log = shader->GetReport();
                if (!log.empty())
                    gfxlog.info("Shader Info: {}")(log);
            }
        }

        // Create shader program which is used as composite
        LLGL::ShaderProgramDescriptor shaderProgramDesc;
        {
            shaderProgramDesc.vertexShader      = vertShader;
            shaderProgramDesc.fragmentShader    = fragShader;
        }
        LLGL::ShaderProgram* shaderProgram = llgl->CreateShaderProgram(shaderProgramDesc);

        // Link shader program and check for errors
        if(shaderProgram->HasErrors())
        {
            gfxlog.error("Shader errors:")
            .note(shaderProgram->GetReport());
            throw Envy::assertion {"Failed to compile shaders"};
        }

        // Create graphics pipeline

        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.shaderProgram                  = shaderProgram;
            pipelineDesc.renderPass                     = context->GetRenderPass();
        }

        // Create graphics PSO
        pipeline = llgl->CreatePipelineState(pipelineDesc);
    }


    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ Envy::graphics ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


    //**********************************************************************
    void init(const description& gfxdesc)
    {

        // -- init LLGL

        llgl = LLGL::RenderSystem::Load("Direct3D11");

        surface = std::make_shared<surface_t>();

        LLGL::RenderContextDescriptor contextdesc {};

        context = llgl->CreateRenderContext(contextdesc, surface);

        // Print renderer information
        const auto& info = llgl->GetRendererInfo();
        gfxlog.info("Render Information")
        .note("Renderer:         {}", info.rendererName)
        .note("Device:           {}", info.deviceName)
        .note("Vendor:           {}", info.vendorName)
        .note("Shading Language: {}", info.shadingLanguageName);

        // Get command queue to record and submit command buffers
        queue = llgl->GetCommandQueue();

        // Create command buffer to submit subsequent graphics commands to the GPU
        commands = llgl->CreateCommandBuffer();

        // -- woot

        Envy::info("Graphics successfully initialized!")();

        // -- event callbacks

        Envy::register_callback<window::resized>(resize);

        triangle();
    }


    //**********************************************************************
    void shutdown()
    {
        llgl->Release(*context);
        llgl->Release(*commands);
    }


    //**********************************************************************
    void clear()
    {
        commands->Begin();
        commands->SetViewport(context->GetResolution());
        commands->BeginRenderPass(*context);
        commands->Clear(LLGL::ClearFlags::Color);

        // -- temp

        commands->SetPipelineState(*pipeline);
        commands->SetVertexBuffer(*vertexBuffer);
        commands->Draw(3, 0);
    }


    //**********************************************************************
    void present()
    {
        commands->EndRenderPass();
        commands->End();
        queue->Submit(*commands);
        context->Present();
    }


    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ Helper Functions ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


    //**********************************************************************
    void resize(const window::resized& resize)
    {
    }


    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ Envy::graphics::surface_t ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


    //**********************************************************************
    surface_t::surface_t()
    {}


    //**********************************************************************
    bool surface_t::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) const
    {
        gfxlog.info("{func}");

        if(nativeHandleSize == sizeof(LLGL::NativeHandle)) {
            auto handle = reinterpret_cast<LLGL::NativeHandle*>(nativeHandle);
            handle->window = window::get_hwnd();
            return true;
        }
        return false;
    }


    //**********************************************************************
    LLGL::Extent2D surface_t::GetContentSize() const
    {
        gfxlog.info("{func}");

        auto winsize {window::get_size()};

        return { (uint32_t) winsize.x, (uint32_t) winsize.y };
    }


    //**********************************************************************
    bool surface_t::AdaptForVideoMode(LLGL::VideoModeDescriptor& videoModeDesc)
    {
        gfxlog.info("{func}");
        gfxlog.warning("{func} is not fully implemented");

        // TODO: Implement fully

        window::request_fullscreen(videoModeDesc.fullscreen);

        // -- unimplemented

        auto winsize {window::get_size()};
        videoModeDesc.resolution = GetContentSize();

        return false;
    }


    //**********************************************************************
    void surface_t::ResetPixelFormat()
    {
        gfxlog.info("{func}");
        gfxlog.warning("{func} is not implemented");
    }


    //**********************************************************************
    bool surface_t::ProcessEvents()
    {
        gfxlog.info("{func}");
        gfxlog.warning("{func} is not implemented");
        return true;
    }


    //**********************************************************************
    std::unique_ptr<LLGL::Display> surface_t::FindResidentDisplay() const
    {
        gfxlog.info("{func}");
        gfxlog.warning("{func} is not implemented");
        return {};
    }

}