#include <graphics.hpp>

#include <window.hpp>
#include <event.hpp>
#include <exception.hpp>

#include <algorithm>

#include <LLGL/LLGL.h>
#include <LLGL/Strings.h>
#include <LLGL/Log.h>
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

        LLGL::VideoModeDescriptor GetVideoMode() const;
    };


    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ TU locals ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


    namespace
    {
        // -- LLGL state

        std::unique_ptr<LLGL::RenderSystem> llgl;
        std::unique_ptr<LLGL::RenderingDebugger> llgl_debug   {nullptr};
        std::unique_ptr<LLGL::RenderingProfiler> llgl_profile {nullptr};

        LLGL::RenderContext* context;
        LLGL::CommandQueue* queue;
        LLGL::CommandBuffer* commands;

        std::shared_ptr<surface_t> surface;

        // -- Diagnostics

        Envy::logger gfxlog {"Graphics"};
    }


    // [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ Helper Forwards ]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


    void resize(const window::resized& resize);

    void llgl_log(LLGL::Log::ReportType type, const std::string& message, const std::string& contextInfo, void* userData);


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
            { {  0,  s }, { 100, 50, 50, 255 } }, // 1st vertex: center-top, red
            { {  s, -s }, { 50, 100, 50, 255 } }, // 2nd vertex: right-bottom, green
            { { -s, -s }, { 50, 50, 100, 255 } }, // 3rd vertex: left-bottom, blue
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
            if(shader != nullptr)
            {
                std::string log = shader->GetReport();
                if (!log.empty())
                    gfxlog.info("Shader Error").note(log);
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

        // -- diagnostic setup

        LLGL::Log::SetReportCallback(llgl_log);
        LLGL::Log::PostReport(LLGL::Log::ReportType::Information, "LLGL Reports will be displayed here");

        // -- init LLGL

        LLGL::RenderSystemDescriptor renderdesc {"Direct3D11"};

        if constexpr (Envy::debug)
        {
            llgl_profile.reset(new LLGL::RenderingProfiler());
            llgl_debug.reset(new LLGL::RenderingDebugger());

            renderdesc.debugCallback = [](const std::string& type, const std::string& message)
            { llgl_log(LLGL::Log::ReportType::Error, message, type, nullptr); };

            if(llgl_profile)
            {
                llgl_profile->timeRecordingEnabled = true;
                gfxlog.info("LLGL Profiling enabled");
            }
        }

        llgl = LLGL::RenderSystem::Load(renderdesc, llgl_profile.get(), llgl_debug.get());

        surface = std::make_shared<surface_t>();

        LLGL::RenderContextDescriptor contextdesc {};
        contextdesc.videoMode = surface->GetVideoMode();

        context = llgl->CreateRenderContext(contextdesc, surface);

        // Print renderer information
        const auto& info = llgl->GetRendererInfo();
        gfxlog.info("Render Information")
        .note("Renderer:         {}", info.rendererName)
        .note("Device:           {}", info.deviceName)
        .note("Vendor:           {}", info.vendorName)
        .note("Shading Language: {}", info.shadingLanguageName);

        // print contex information
        const auto resolution = context->GetResolution();
        gfxlog.info("Context Information")
        .note("resolution:         {} x {}", resolution.width, resolution.height)
        .note("samples:            {}", context->GetSamples())
        .note("colorFormat:        {}", LLGL::ToString(context->GetColorFormat()))
        .note("depthStencilFormat: {}", LLGL::ToString(context->GetDepthStencilFormat()));

        // Get command queue to record and submit command buffers
        queue = llgl->GetCommandQueue();

        // Create command buffer to submit subsequent graphics commands to the GPU
        LLGL::CommandBufferDescriptor commandsdesc {};
        commands = llgl->CreateCommandBuffer(commandsdesc);
        commands->SetClearColor({0.3f,0.3f,0.3f,1.0f});

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
        if(llgl_profile && llgl_profile->timeRecordingEnabled)
        {
            llgl_profile->NextProfile();
        }

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


    //**********************************************************************
    void llgl_log(LLGL::Log::ReportType type, const std::string& message, const std::string& contextInfo, void* userData)
    {
        Envy::log::severity sev;

        switch(type)
        {
            using enum Envy::log::severity;
            using enum LLGL::Log::ReportType;

            case Error:        sev = error;    break;
            case Warning:      sev = warning;  break;
            case Information:  sev = info;     break;
            case Performance:  sev = warning;  break;
        }

        Envy::log::update_log_state(gfxlog.get_name(), sev, Envy::log::message_source {});
        Envy::log::raw_log(gfxlog.get_file(), gfxlog.logs_to_console(), std::format("{} : {}", contextInfo, message));
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

    //**********************************************************************
    LLGL::VideoModeDescriptor surface_t::GetVideoMode() const
    {
        LLGL::VideoModeDescriptor videomode {};

        // videomode.colorBits;
        // videomode.depthBits;
        // videomode.stencilBits;

        videomode.fullscreen = window::is_fullscreen();
        videomode.resolution = GetContentSize();
        videomode.swapChainSize = 3;;

        return videomode;
    }

}