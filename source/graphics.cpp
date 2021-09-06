#include <graphics.hpp>

#include <window.hpp>
#include <event.hpp>

// Diligent
#include "EngineFactoryD3D11.h"
#include "EngineFactoryD3D12.h"

#include <algorithm>

#if defined(max)
#undef max
#endif

namespace Envy::graphics
{

    namespace
    {
        // state
        Diligent::IRenderDevice*  device;
        Diligent::IDeviceContext* context;
        Diligent::ISwapChain*     swapchain;

        Envy::vector2<u32> buffer_size;
    }


    void resize(const window::resized& resize);


    void diligent_log_callback(Diligent::DEBUG_MESSAGE_SEVERITY Severity, const Diligent::Char *Message, const Diligent::Char *Function, const Diligent::Char *File, int Line)
    {
        Envy::log::severity severity;

        switch(Severity)
        {
            case Diligent::DEBUG_MESSAGE_SEVERITY_INFO: severity = log::severity::info; break;
            case Diligent::DEBUG_MESSAGE_SEVERITY_WARNING: severity = log::severity::warning; break;
            case Diligent::DEBUG_MESSAGE_SEVERITY_ERROR: severity = log::severity::error; break;
            case Diligent::DEBUG_MESSAGE_SEVERITY_FATAL_ERROR: severity = log::severity::error; break;
        }

        Envy::log::message_source src {};

        if(File)     src.file = File;
        if(Function) src.func = Function;
        if(Line > 0) src.line = Line;

        Envy::log::update_log_state("Diligent", severity, src);
        Envy::log::raw_log("", true, std::string_view(Message?Message:""));
    }


    void init(const description& gfxdesc)
    {
        Envy::scope_logger sl("Initializing Diligent");

        Diligent::SetDebugMessageCallback(diligent_log_callback);

        Diligent::SwapChainDesc swap_chain_desc;

        Diligent::EngineD3D12CreateInfo diligent_ci;

        auto* factory {Diligent::GetEngineFactoryD3D12()};

        factory->CreateDeviceAndContextsD3D12(diligent_ci, &device, &context);

        Envy::assert(device, "Could not create Render Device!");
        Envy::assert(context, "Could not create Device Context!");

        Envy::assert(window::get_hwnd(), "Window must be initialized before Diligent SwapChain can be created!");

        factory->CreateSwapChainD3D12(device, context, swap_chain_desc, Diligent::FullScreenModeDesc{}, Diligent::Win32NativeWindow{window::get_hwnd()}, &swapchain);

        Envy::assert(swapchain, "Could not create SwapChain!");

        swap_chain_desc = swapchain->GetDesc();
        buffer_size = { swap_chain_desc.Width, swap_chain_desc.Height };

        Envy::info("Graphics successfully initialized!")();

        // event callbacks

        Envy::register_callback<window::resized>(resize);
    }


    static const char* VSSource = R"(
    struct PSInput
    {
        float4 Pos   : SV_POSITION;
        float3 Color : COLOR;
    };
    void main(in  uint    VertId : SV_VertexID,
            out PSInput PSIn)
    {
        float4 Pos[3];
        Pos[0] = float4(-0.5, -0.5, 0.0, 1.0);
        Pos[1] = float4( 0.0, +0.5, 0.0, 1.0);
        Pos[2] = float4(+0.5, -0.5, 0.0, 1.0);
        float3 Col[3];
        Col[0] = float3(1.0, 0.0, 0.0); // red
        Col[1] = float3(0.0, 1.0, 0.0); // green
        Col[2] = float3(0.0, 0.0, 1.0); // blue
        PSIn.Pos   = Pos[VertId];
        PSIn.Color = Col[VertId];
    }
    )";

    static const char* PSSource = R"(
    struct PSInput
    {
        float4 Pos   : SV_POSITION;
        float3 Color : COLOR;
    };
    struct PSOutput
    {
        float4 Color : SV_TARGET;
    };
    void main(in  PSInput  PSIn,
            out PSOutput PSOut)
    {
        PSOut.Color = float4(PSIn.Color.rgb, 1.0);
    }
    )";


    Diligent::IPipelineState* triangle()
    {
        // Pipeline state object encompasses configuration of all GPU stages

        static Diligent::IPipelineState* pipeline {};

        Diligent::GraphicsPipelineStateCreateInfo psci;

        // Pipeline state name is used by the engine to report issues.
        // It is always a good idea to give objects descriptive names.
        psci.PSODesc.Name = "Simple triangle PSO";

        // This is a graphics pipeline
        psci.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;

        // clang-format off
        // This tutorial will render to a single render target
        psci.GraphicsPipeline.NumRenderTargets             = 1;
        // Set render target format which is the format of the swap chain's color buffer
        psci.GraphicsPipeline.RTVFormats[0]                = swapchain->GetDesc().ColorBufferFormat;
        // Use the depth buffer format from the swap chain
        psci.GraphicsPipeline.DSVFormat                    = swapchain->GetDesc().DepthBufferFormat;
        // Primitive topology defines what kind of primitives will be rendered by this pipeline state
        psci.GraphicsPipeline.PrimitiveTopology            = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        // No back face culling for this tutorial
        psci.GraphicsPipeline.RasterizerDesc.CullMode      = Diligent::CULL_MODE_NONE;
        // Disable depth testing
        psci.GraphicsPipeline.DepthStencilDesc.DepthEnable = Diligent::False;
        // clang-format on

        Diligent::ShaderCreateInfo shaderci;
        // Tell the system that the shader source code is in HLSL.
        // For OpenGL, the engine will convert this into GLSL under the hood
        shaderci.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
        // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
        shaderci.UseCombinedTextureSamplers = true;
        // Create a vertex shader
        Diligent::IShader* pVS {nullptr};
        {
            shaderci.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
            shaderci.EntryPoint      = "main";
            shaderci.Desc.Name       = "Triangle vertex shader";
            shaderci.Source          = VSSource;
            device->CreateShader(shaderci, &pVS);
        }

        // Create a pixel shader
        Diligent::IShader* pPS {nullptr};
        {
            shaderci.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
            shaderci.EntryPoint      = "main";
            shaderci.Desc.Name       = "Triangle pixel shader";
            shaderci.Source          = PSSource;
            device->CreateShader(shaderci, &pPS);
        }

        // Finally, create the pipeline state
        psci.pVS = pVS;
        psci.pPS = pPS;
        device->CreateGraphicsPipelineState(psci, &pipeline);

        return pipeline;
    }


    void clear()
    {
        const float clearcolor[] = {  0.1f,  0.1f,  0.1f, 1.0f };

        auto* rtv = swapchain->GetCurrentBackBufferRTV();
        auto* dsv = swapchain->GetDepthBufferDSV();

        context->SetRenderTargets(1, &rtv, dsv, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        context->ClearRenderTarget(rtv, clearcolor, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        context->ClearDepthStencil(dsv, Diligent::CLEAR_DEPTH_FLAG, 1.f, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    }


    void present()
    {
        static Diligent::IPipelineState* pso { triangle() };
        context->SetPipelineState(pso);

        Diligent::DrawAttribs drawAttrs;
        drawAttrs.NumVertices = 3; // Render 3 vertices
        context->Draw(drawAttrs);

        swapchain->Present();
    }


    void resize(const window::resized& resize)
    {

        if(resize.size.x > (i32) buffer_size.x || resize.size.y > (i32) buffer_size.y)
        {
            u32 w { std::max( (u32) resize.size.x, buffer_size.x) };
            u32 h { std::max( (u32) resize.size.y, buffer_size.y) };

            swapchain->Resize(w, h);

            auto desc { swapchain->GetDesc() };
            buffer_size = { desc.Width, desc.Height };
        }

        Envy::info("Resize Event!")
            .note("window size: {}", resize.size.to_string())
            .note("buffer size: {}", buffer_size.to_string());
    }

}