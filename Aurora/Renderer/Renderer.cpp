#include "Aurora.h"
#include "Renderer.h"
#include "../Window/WindowContext.h"
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3dcompiler.lib")

namespace Aurora
{
    Renderer::Renderer(EngineContext* engineContext) : ISubsystem(engineContext)
    {
    }

    bool Renderer::Initialize()
    {
        // Swapchain Creation
        DXGI_SWAP_CHAIN_DESC swapchainInfo = {};
        swapchainInfo.BufferDesc.RefreshRate.Numerator = 0;
        swapchainInfo.BufferDesc.RefreshRate.Denominator = 1;
        swapchainInfo.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        swapchainInfo.BufferDesc.Width = 1280;
        swapchainInfo.BufferDesc.Height = 720;
        swapchainInfo.SampleDesc.Count = 1;
        swapchainInfo.SampleDesc.Quality = 0;
        swapchainInfo.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapchainInfo.BufferCount = 1; // Amount of backbuffers. 1 front buffer and 1 back buffer.
        swapchainInfo.OutputWindow = m_EngineContext->GetSubsystem<WindowContext>()->GetWindowHWND(0);
        swapchainInfo.Windowed = true;

        // Create device and swapchain.
        D3D_FEATURE_LEVEL featureLevel;
        UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_DEBUG;
        HRESULT result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, nullptr, 0, D3D11_SDK_VERSION, &swapchainInfo, &m_SwapChain, &m_Device, &featureLevel, &m_DeviceContext);

        AURORA_ASSERT(S_OK == result && m_SwapChain && m_Device && m_DeviceContext);

        // Create render target view.
        // The output images from Direct3D are called render targets. We can retrieve a view pointer to ours by fetching it from our swapchain.
        ID3D11Texture2D* framebuffer;
        result = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&framebuffer);
        AURORA_ASSERT(SUCCEEDED(result));

        result = m_Device->CreateRenderTargetView(framebuffer, 0, &m_RenderTargetView);
        AURORA_ASSERT(SUCCEEDED(result));
        framebuffer->Release();

        CompileShaders();
        CreateBuffers();

        return true;
    }

    void Renderer::Tick(float deltaTime)
    {
        // Clear the backbuffer to black for the new frame.
        float backgroundColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        m_DeviceContext->ClearRenderTargetView(m_RenderTargetView, backgroundColor); // We use the render target view pointer to access the back buffer and clear it to an RGBA color of our choice (values between 0 and 1). If we add a depth buffer later, we will need to clear it as well.

        // Viewport
        D3D11_VIEWPORT viewportInfo = { 0, 0, m_EngineContext->GetSubsystem<WindowContext>()->GetWindowWidth(0), m_EngineContext->GetSubsystem<WindowContext>()->GetWindowHeight(0), 0.0f, 1.0f };
        m_DeviceContext->RSSetViewports(1, &viewportInfo);
        m_DeviceContext->OMSetRenderTargets(1, &m_RenderTargetView, nullptr); // Set depth as well here if it exists.

        // Update input assembler with the vertex buffer to draw, and the memory layout so it knows how to feed vertex data from the vertex buffer to the vertex shader.
        m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // Expects vertices should form a triangle from 3 vertices.
        m_DeviceContext->IASetInputLayout(m_InputLayout); // Give it our input layout.
        m_DeviceContext->IASetVertexBuffers(0, 1, &m_VertexBuffer, &m_VertexStride, &m_VertexOffset); // Tell it to use our vertex buffer with the earlier set memory stride anmd offset from our data.
        m_DeviceContext->VSSetShader(m_VertexShader, nullptr, 0);
        m_DeviceContext->PSSetShader(m_PixelShader, nullptr, 0);

        // When we call Draw, the pipeline will use all the states we just set, the vertex buffer and the shaders. We also need to tell it how many vertices to draw from our buffer.
        m_DeviceContext->Draw(m_VertexCount, 0);

        // Swap buffers. DXGI calls this Present(). This should be after all of our draw calls. This will be from our swapchain.
        m_SwapChain->Present(1, 0);

        m_DeviceContext->VSSetShader(nullptr, nullptr, 0);
        m_DeviceContext->PSSetShader(nullptr, nullptr, 0);
    }                                 

    void Renderer::CompileShaders()
    {
        /// Compile shaders. Returns a compiled blob (binary large object) for each shader. Can also capture error blobs we can use to print error messages in case our shaders don't compile.
        UINT flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG;
        ID3DBlob* vsBlob = nullptr, * psBlob = nullptr, * errorBlob = nullptr;

        // Compile Vertex Shader
        HRESULT result = D3DCompileFromFile(L"../Resources/Shaders/Triangle.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "vs_main", "vs_5_0", flags, 0, &vsBlob, &errorBlob);
        
        if(FAILED(result))
        {
            if (errorBlob)
            {
                AURORA_ERROR((char*)errorBlob->GetBufferPointer());
                errorBlob->Release();
            }

            if (vsBlob)
            {
                vsBlob->Release();
                AURORA_ASSERT(false);
            }
        }

        // Compile Pixel Shader
        result = D3DCompileFromFile(L"../Resources/Shaders/Triangle.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "ps_main", "ps_5_0", flags, 0, &psBlob, &errorBlob);
        if (FAILED(result))
        {
            if (errorBlob)
            {
                AURORA_ERROR((char*)errorBlob->GetBufferPointer());
                errorBlob->Release();
            }

            if (psBlob)
            {
                psBlob->Release();
                AURORA_ASSERT(false);
            }
        }

        result = m_Device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_VertexShader);
        AURORA_ASSERT(SUCCEEDED(result));

        result = m_Device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_PixelShader);
        AURORA_ASSERT(SUCCEEDED(result));

        // We now need an input layout to describe how vertex data memory from a buffer should map to the input variables for the vertex shaders.
        D3D11_INPUT_ELEMENT_DESC inputElementDescription[] = {
            { "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // We only have 1 input variable to the vertex shader - the XYZ position. Here, our XYZ has 3 components. Each element is a 32-bit float. This corresponds to DXGI_FORMAT_R32G32B32_FLOAT and will appear as a float3 in our shader. A float4 may be R32G32B32A42_FLOAT.
        };

        result = m_Device->CreateInputLayout(inputElementDescription, ARRAYSIZE(inputElementDescription), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_InputLayout);
        AURORA_ASSERT(SUCCEEDED(result));
    }

    void Renderer::CreateBuffers()
    {
        // Clockwise winding order. We will define them in visible clipspace as we won't be using matrixes yet. (XY: -1 to 1, Z: 0 to 1).
        float vertexDataArray[]{
            0.0f, 0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            -0.5f, -0.5f, 0.0f,
        };

        m_VertexStride = 3 * sizeof(float);
        m_VertexOffset = 0;
        m_VertexCount = 3;

        D3D11_BUFFER_DESC vertexBufferInfo = {}; // Parameters that describe the bytes between each vertex (XYZ, so 3 floats), the offset into the buffer to start reading (0), and the numbver of vertices (3).
        vertexBufferInfo.ByteWidth = sizeof(vertexDataArray);
        vertexBufferInfo.Usage = D3D11_USAGE_DEFAULT; // Based on how the buffer values will change, there are many ways to help driver optimization, such as IMMUTABLE or DEFAULT. Different usage values will require struct fields to be set or not set.
        vertexBufferInfo.BindFlags = D3D11_BIND_VERTEX_BUFFER; // We are binding a vertex buffer here.

        D3D11_SUBRESOURCE_DATA subresourceData = {}; // Points to the actual vertex array data.
        subresourceData.pSysMem = vertexDataArray;

        HRESULT result = m_Device->CreateBuffer(&vertexBufferInfo, &subresourceData, &m_VertexBuffer);
    }
}