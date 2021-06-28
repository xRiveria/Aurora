#include "Aurora.h"
#include "Renderer.h"
#include "../Window/WindowContext.h"
#include "../Graphics/DX11/DX11_Utilities.h" //Temporary
#include "../Scene/World.h"

namespace Aurora
{
    Renderer::Renderer(EngineContext* engineContext) : ISubsystem(engineContext)
    {

    }

    Renderer::~Renderer()
    {

    }

    bool Renderer::Initialize()
    {
        m_GraphicsDevice = std::make_shared<DX11_GraphicsDevice>(m_EngineContext, true);

        RHI_SwapChain_Description swapchainDescription;
        swapchainDescription.m_Width = static_cast<uint32_t>(m_EngineContext->GetSubsystem<WindowContext>()->GetWindowWidth(0));
        swapchainDescription.m_Height = static_cast<uint32_t>(m_EngineContext->GetSubsystem<WindowContext>()->GetWindowHeight(0));

        m_GraphicsDevice->CreateSwapChain(&swapchainDescription, &m_SwapChain);

        CompileShaders();
        CreateBuffers();
        CreateDepth();
        CreateTexture();
        CreateRasterizerStates();

        m_Camera = std::make_shared<Camera>(m_EngineContext);
        m_Camera->SetPosition(0.0f, 0.0f, -2.0f);
        m_Camera->ComputePerspectiveMatrix(90.0f, static_cast<float>(m_EngineContext->GetSubsystem<WindowContext>()->GetWindowWidth(0)) / static_cast<float>(m_EngineContext->GetSubsystem<WindowContext>()->GetWindowHeight(0)), 0.1f, 1000.0f);

        m_EngineContext->GetSubsystem<World>()->LoadModel("../Resources/Models/Hollow_Knight/v3.obj");
        return true;
    }

    void Renderer::Tick(float deltaTime)
    {
        auto internalState = DX11_Utility::ToInternal(&m_SwapChain);

        // Clear the backbuffer to black for the new frame.
        float backgroundColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

        ID3D11RenderTargetView* RTV = internalState->m_RenderTargetView.Get();
        ID3D11DepthStencilView* DSV = DX11_Utility::ToInternal(&m_DepthTexture)->m_DepthStencilView.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->ClearRenderTargetView(RTV, backgroundColor); // We use the render target view pointer to access the back buffer and clear it to an RGBA color of our choice (values between 0 and 1). If we add a depth buffer later, we will need to clear it as well.
        m_GraphicsDevice->m_DeviceContextImmediate->ClearDepthStencilView(DSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        // Viewport
        D3D11_VIEWPORT viewportInfo = { 0, 0, m_EngineContext->GetSubsystem<WindowContext>()->GetWindowWidth(0), m_EngineContext->GetSubsystem<WindowContext>()->GetWindowHeight(0), 0.0f, 1.0f };
        m_GraphicsDevice->m_DeviceContextImmediate->RSSetViewports(1, &viewportInfo);

        m_GraphicsDevice->m_DeviceContextImmediate->OMSetRenderTargets(1, &RTV, DSV); // Set depth as well here if it exists.
        m_GraphicsDevice->m_DeviceContextImmediate->OMSetDepthStencilState(m_DepthStencilState.Get(), 0);
        //m_GraphicsDevice->m_DeviceContextImmediate->RSSetState(m_RasterizerState_Wireframe);

        // Update input assembler with the vertex buffer to draw, and the memory layout so it knows how to feed vertex data from the vertex buffer to the vertex shader.
        m_GraphicsDevice->m_DeviceContextImmediate->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // Expects vertices should form a triangle from 3 vertices.
        m_GraphicsDevice->m_DeviceContextImmediate->IASetInputLayout(m_InputLayout); // Give it our input layout.

        ID3D11SamplerState* samplerState = DX11_Utility::ToInternal(&m_Standard_Texture_Sampler)->m_Resource.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->PSSetSamplers(0, 1, &samplerState);

        ID3D11ShaderResourceView* shaderResourceView = DX11_Utility::ToInternal(&m_PyramidTexture->m_Texture)->m_ShaderResourceView.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->PSSetShaderResources(0, 1, &shaderResourceView);

        ID3D11Buffer* vertexBuffer = (ID3D11Buffer*)DX11_Utility::ToInternal(&m_VertexBuffer)->m_Resource.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->IASetVertexBuffers(0, 1, &vertexBuffer, &m_VertexStride, &m_VertexOffset); // Tell it to use our vertex buffer with the earlier set memory stride anmd offset from our data.
        
        ID3D11VertexShader* vertexShader = static_cast<DX11_Utility::DX11_VertexShaderPackage*>(m_VertexShader.m_InternalState.get())->m_Resource.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->VSSetShader(vertexShader, nullptr, 0);

        ID3D11PixelShader* pixelShader = static_cast<DX11_Utility::DX11_PixelShaderPackage*>(m_PixelShader.m_InternalState.get())->m_Resource.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->PSSetShader(pixelShader, nullptr, 0);

        //==========================================================================================================================
        // Update Transform Component
        m_Transform.m_ModelMatrix = XMMatrixScaling(0.2f, 0.2f, 0.2f); // XMMatrixRotationRollPitchYaw(0.0f, 0.0f, zRotation) * XMMatrixScaling(0.5f, 0.5f, 1.0f) * XMMatrixTranslation(0.0f, 0.0f, 0.0f);
        // m_Camera->ComputeLookAtPosition(XMFLOAT3(0.0f, 0.0f, 0.0f)); // Set our camera to stare at a position. 

        transform.m_MVP = m_Transform.m_ModelMatrix * m_Camera->GetViewMatrix() * m_Camera->GetProjectionMatrix();
        transform.m_MVP = XMMatrixTranspose(transform.m_MVP);

        m_ConstantBufferMapping.m_Flags = Mapping_Flag::Flag_Write;
        m_GraphicsDevice->Map(&m_ConstantBuffer_VertexTransform, &m_ConstantBufferMapping);
        CopyMemory(m_ConstantBufferMapping.m_Data, &transform, sizeof(CB_VertexShader));
        m_GraphicsDevice->Unmap(&m_ConstantBuffer_VertexTransform);

        ID3D11Buffer* constantBuffer = (ID3D11Buffer*)DX11_Utility::ToInternal(&m_ConstantBuffer_VertexTransform)->m_Resource.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->VSSetConstantBuffers(0, 1, &constantBuffer);

        //==========================================================================================================================

        // When we call Draw, the pipeline will use all the states we just set, the vertex buffer and the shaders. We also need to tell it how many vertices to draw from our buffer.
        m_GraphicsDevice->m_DeviceContextImmediate->Draw(m_VertexCount, 0);

        DrawModel();

        // Swap buffers. DXGI calls this Present(). This should be after all of our draw calls. This will be from our swapchain.

       // m_GraphicsDevice->m_DeviceContextImmediate->VSSetShader(nullptr, nullptr, 0);
       // m_GraphicsDevice->m_DeviceContextImmediate->PSSetShader(nullptr, nullptr, 0);

       Present();
       m_Camera->Tick(deltaTime);
    }

    void Renderer::DrawModel()
    {
        World* world = m_EngineContext->GetSubsystem<World>();
        MeshComponent meshComponent = world->m_MeshComponent;

        UINT offset = 0;
        UINT modelStride = 5 * sizeof(float);

        ID3D11ShaderResourceView* shaderResourceView = DX11_Utility::ToInternal(&meshComponent.m_BaseTexture->m_Texture)->m_ShaderResourceView.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->PSSetShaderResources(0, 1, &shaderResourceView);

        ID3D11Buffer* vertexBuffer = (ID3D11Buffer*)DX11_Utility::ToInternal(&meshComponent.m_VertexBuffer_Position)->m_Resource.Get();
        m_GraphicsDevice->m_DeviceContextImmediate->IASetVertexBuffers(0, 1, &vertexBuffer, &modelStride, &offset);
        //ID3D11Buffer* indexBuffer = (ID3D11Buffer*)DX11_Utility::ToInternal(&meshComponent.m_IndexBuffer)->m_Resource.Get();
        //m_GraphicsDevice->m_DeviceContextImmediate->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
        m_GraphicsDevice->m_DeviceContextImmediate->Draw((UINT)meshComponent.m_VertexPositions.size(), 0);
    }

    void Renderer::Present()
    {
        auto internalState = DX11_Utility::ToInternal(&m_SwapChain);
        internalState->m_SwapChain->Present(1, 0);
    }

    void Renderer::CompileShaders()
    {
        /// Compile shaders. Returns a compiled blob (binary large object) for each shader. Can also capture error blobs we can use to print error messages in case our shaders don't compile.
        UINT flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG;
        ID3DBlob* vsBlob = nullptr, * psBlob = nullptr, * errorBlob = nullptr;

        // Compile Vertex RHI_Shader
        if (!DX11_Utility::BreakIfFailed(D3DCompileFromFile(L"../Resources/Shaders/Triangle.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "vs_main", "vs_5_0", flags, 0, &vsBlob, &errorBlob)))
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

        // Compile Pixel RHI_Shader
        if (!DX11_Utility::BreakIfFailed(D3DCompileFromFile(L"../Resources/Shaders/Triangle.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "ps_main", "ps_5_0", flags, 0, &psBlob, &errorBlob)))
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

        m_GraphicsDevice->CreateShader(ShaderStage::Vertex_Shader, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_VertexShader);
        m_GraphicsDevice->CreateShader(ShaderStage::Pixel_Shader, psBlob->GetBufferPointer(), psBlob->GetBufferSize(), &m_PixelShader);

        // We now need an input layout to describe how vertex data memory from a buffer should map to the input variables for the vertex shaders.
        D3D11_INPUT_ELEMENT_DESC inputElementDescription[] = {
            { "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // We only have 1 input variable to the vertex shader - the XYZ position. Here, our XYZ has 3 components. Each element is a 32-bit float. This corresponds to DXGI_FORMAT_R32G32B32_FLOAT and will appear as a float3 in our shader. A float4 may be R32G32B32A42_FLOAT.
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
        };

        DX11_Utility::BreakIfFailed(m_GraphicsDevice->m_Device->CreateInputLayout(inputElementDescription, ARRAYSIZE(inputElementDescription), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_InputLayout));
    }

    void Renderer::CreateBuffers()
    {
        // Clockwise winding order. We will define them in visible clipspace as we won't be using matrixes yet. (XY: -1 to 1, Z: 0 to 1).
        float vertexDataArray[]{
            -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 0.5f, 0.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 1.0f
        };

        m_VertexStride = 5 * sizeof(float);
        m_VertexOffset = 0;
        m_VertexCount = 3;

        RHI_GPU_Buffer_Description bufferDescription;
        bufferDescription.m_BindFlags = Bind_Flag::Bind_Vertex_Buffer;
        bufferDescription.m_Usage = Usage::Default;
        bufferDescription.m_ByteWidth = sizeof(vertexDataArray);

        RHI_Subresource_Data subresourceData;
        subresourceData.m_SystemMemory = vertexDataArray;

        m_GraphicsDevice->CreateBuffer(&bufferDescription, &subresourceData, &m_VertexBuffer);

        // HLSL is column major. DX11 Math library is row major. Thus, we transpose before sending the data to the shader.
        RHI_GPU_Buffer_Description constantBufferDescription;
        constantBufferDescription.m_BindFlags = Bind_Flag::Bind_Constant_Buffer;
        constantBufferDescription.m_Usage = Usage::Dynamic;
        constantBufferDescription.m_CPUAccessFlags = CPU_Access::CPU_Access_Write;
        constantBufferDescription.m_ByteWidth = sizeof(CB_VertexShader);
        constantBufferDescription.m_StructureByteStride = 0;

        m_GraphicsDevice->CreateBuffer(&constantBufferDescription, nullptr, &m_ConstantBuffer_VertexTransform);
    }

    void Renderer::CreateRasterizerStates()
    {
        D3D11_RASTERIZER_DESC wireframeState = {};
        wireframeState.FillMode = D3D11_FILL_SOLID;
        wireframeState.CullMode = D3D11_CULL_NONE;

        DX11_Utility::BreakIfFailed(m_GraphicsDevice->m_Device->CreateRasterizerState(&wireframeState, &m_RasterizerState_Wireframe));
    }

    void Renderer::CreateDepth()
    {
        RHI_Texture_Description depthStencilDescription;
        depthStencilDescription.m_Type = Texture_Type::Texture2D;
        depthStencilDescription.m_Width = static_cast<uint32_t>(m_EngineContext->GetSubsystem<WindowContext>()->GetWindowWidth(0));
        depthStencilDescription.m_Height = static_cast<uint32_t>(m_EngineContext->GetSubsystem<WindowContext>()->GetWindowHeight(0));
        depthStencilDescription.m_MipLevels = 1;
        depthStencilDescription.m_ArraySize = 1;
        depthStencilDescription.m_Format = Format::FORMAT_D24_UNORM_S8_UINT;
        depthStencilDescription.m_SampleCount = 1;
        depthStencilDescription.m_Usage = Usage::Default;
        depthStencilDescription.m_BindFlags = Bind_Flag::Bind_Depth_Stencil;
        depthStencilDescription.m_CPUAccessFlags = 0;
        depthStencilDescription.m_MiscFlags = 0;

        m_GraphicsDevice->CreateTexture(&depthStencilDescription, nullptr, &m_DepthTexture);

        /// To Abstract
        D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
        depthStencilDesc.DepthEnable = true;
        depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
        depthStencilDesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;

        m_GraphicsDevice->m_Device->CreateDepthStencilState(&depthStencilDesc, m_DepthStencilState.GetAddressOf());
    }

    void Renderer::CreateTexture()
    {
        m_PyramidTexture = m_EngineContext->GetSubsystem<ResourceCache>()->Load("Pyramid.jpg", "../Resources/Textures/Pyramid.jpg");

        RHI_Sampler_Description samplerDescription;
        samplerDescription.m_Filter = Filter::FILTER_MIN_MAG_MIP_LINEAR;
        samplerDescription.m_AddressU = Texture_Address_Mode::Texture_Address_Wrap;
        samplerDescription.m_AddressV = Texture_Address_Mode::Texture_Address_Wrap;
        samplerDescription.m_AddressW = Texture_Address_Mode::Texture_Address_Wrap;
        samplerDescription.m_ComparisonFunction = ComparisonFunction::Comparison_Never;
        samplerDescription.m_MinLOD = 0;
        samplerDescription.m_MaxLOD = D3D11_FLOAT32_MAX;

        m_GraphicsDevice->CreateSampler(&samplerDescription, &m_Standard_Texture_Sampler);
    }
}