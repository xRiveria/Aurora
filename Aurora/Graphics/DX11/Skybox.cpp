#include "Aurora.h"
#include "Skybox.h"
#include "../Scene/World.h"
#include "RenderTexture.h"
#include "../Resource/ResourceCache.h"

namespace Aurora
{
    const int g_SkyboxSize = 2048;

    Skybox::Skybox(EngineContext* engineContext) : m_EngineContext(engineContext)
    {
        m_Renderer = m_EngineContext->GetSubsystem<Renderer>();
    }

    Skybox::~Skybox()
    {
    }

    bool Skybox::InitializeResources()
    {
        // Create Cube Entity
        m_SkyboxEntity = m_EngineContext->GetSubsystem<World>()->CreateDefaultObject(DefaultObjectType::DefaultObjectType_Cube);
        m_SkyboxEntity->m_EntityType = EntityType::Skybox;
        m_SkyboxEntity->SetName("Skybox");

        for (int i = 0; i < m_SkyboxEntity->GetComponent<Transform>()->m_Children.size(); i++)
        {
            m_SkyboxEntity->GetComponent<Transform>()->m_Children[i]->GetEntity()->m_EntityType = EntityType::Skybox;
        }

        // Create Skybox Shader
        m_Renderer->LoadShader(Shader_Stage::Vertex_Shader, m_SkyVertexShader, "SkyboxVS.hlsl");
        m_Renderer->LoadShader(Shader_Stage::Pixel_Shader, m_SkyPixelShader, "SkyboxPS.hlsl");

        CreateCubeMap();

        return true;
    }

    void Skybox::BindMesh() const
    {
        // We are doing this as our actual cube mesh is a child of the root cube.
        Mesh* childMeshComponent = m_SkyboxEntity->GetComponent<Transform>()->m_Children[0]->GetEntity()->GetComponent<Mesh>();

        unsigned int modelStride = 8 * sizeof(float);
        unsigned int offset = 0;

        // Bind Vertex Buffer
        ID3D11Buffer* vertexBuffer = (ID3D11Buffer*)DX11_Utility::ToInternal(&childMeshComponent->m_VertexBuffer_Position)->m_Resource.Get();
        m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->IASetVertexBuffers(0, 1, &vertexBuffer, &modelStride, &offset);

        // Bind Index Buffer
        ID3D11Buffer* indexBuffer = (ID3D11Buffer*)DX11_Utility::ToInternal(&childMeshComponent->m_IndexBuffer)->m_Resource.Get();
        m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->IASetIndexBuffer(indexBuffer, (childMeshComponent->GetIndexFormat() == IndexBuffer_Format::Format_16Bit ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT), 0);

        // Set Primitive Topology
        m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        
        // Update constant buffer data.
        if (childMeshComponent->GetEntity()->HasComponent<Material>())
        {
            m_Renderer->UpdateMaterialConstantBuffer(childMeshComponent->GetEntity()->GetComponent<Material>());
        }
    }

    bool Skybox::CreateCubeMap()
    {
        std::vector<std::shared_ptr<RenderTexture>> cubeFaces;

        // Load initial image.
        m_SkyHDR = m_EngineContext->GetSubsystem<ResourceCache>()->LoadTextureHDR("../Resources/Textures/HDR/Winter_Forest/WinterForest_Env.hdr");

        // Create faces.
        for (int i = 0; i < 6; ++i)
        {
            std::shared_ptr<RenderTexture> renderTexture = std::make_shared<RenderTexture>(m_EngineContext);
            renderTexture->InitializeResources(g_SkyboxSize, g_SkyboxSize, 1);
            cubeFaces.push_back(renderTexture);
        }

        // Create Skybox Shader
        m_Renderer->LoadShader(Shader_Stage::Vertex_Shader, m_RectToCubemapVSShader, "RectToCubemapVS.hlsl");
        m_Renderer->LoadShader(Shader_Stage::Pixel_Shader, m_RectToCubemapPSShader, "RectToCubemapPS.hlsl");

        BindMesh();

        m_Renderer->LoadSkyPipelineState(&m_SkyVertexShader, &m_SkyPixelShader);

        //// Already binding the texture through our constant buffer.

        // Render
        for (int i = 0; i < 6; ++i)
        {
            RenderTexture* texture = cubeFaces[i].get();

            ID3D11DepthStencilView* ourDepthStencilTexture = DX11_Utility::ToInternal(&m_Renderer->m_DepthBuffer_Main)->m_DepthStencilView.Get();
            texture->SetRenderTarget(ourDepthStencilTexture);
            texture->ClearRenderTarget(ourDepthStencilTexture, { 0, 0, 1 });

            //if (i == 0) m_Renderer->m_Camera->GetComponent<Camera>()->SetRotation(0.0f, 90.0f, 0.0f); // front
            //if (i == 1) m_Renderer->m_Camera->GetComponent<Camera>()->SetRotation(0.0f, 270.0f, 0.0f); // back
            //if (i == 2) m_Renderer->m_Camera->GetComponent<Camera>()->SetRotation(-90.0f, 0.0f, 0.0f); // top
            //if (i == 3) m_Renderer->m_Camera->GetComponent<Camera>()->SetRotation(90.0f, 0.0f, 0.0f); // bottom
            //if (i == 4) m_Renderer->m_Camera->GetComponent<Camera>()->SetRotation(0.0f, 0.0f, 0.0f); // left
            //if (i == 5) m_Renderer->m_Camera->GetComponent<Camera>()->SetRotation(0.0f, 180.0f, 0.0f); // right

            // Transform Rect to Cubemap
            RenderForResources();
        }

        m_Cubemap = new Cubemap(m_EngineContext);
        if (!m_Cubemap->InitializeResources(cubeFaces, g_SkyboxSize, g_SkyboxSize, 1))
        {
            return false;
        }

        /// Irradiance
        /// Pre-Filter
        /// Integrate BRDF
        
        /// Reset Camera

        return true;
    }

    bool Skybox::Render() const
    {
        // Bind Vertex, Index and Primitive Topologies
        BindMesh();

        const RHI_PipelineState* pipelineStateObject = &m_Renderer->m_PSO_Object_Sky;
        const RHI_PipelineState_Description& pipelineDescription = pipelineStateObject != nullptr ? pipelineStateObject->GetDescription() : RHI_PipelineState_Description();

        auto internalState = DX11_Utility::ToInternal(pipelineStateObject);

        // Bind Input Layout
        ID3D11InputLayout* inputLayout = pipelineDescription.m_InputLayout == nullptr ? nullptr : internalState->m_InputLayout.Get();
        m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->IASetInputLayout(inputLayout);
        
        // Bind Sky Vertex Shader
        ID3D11VertexShader* vertexShader = pipelineDescription.m_VertexShader == nullptr ? nullptr : static_cast<DX11_Utility::DX11_VertexShaderPackage*>(pipelineDescription.m_VertexShader->m_InternalState.get())->m_Resource.Get();
        m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->VSSetShader(vertexShader, nullptr, 0);

        // Bind Sky Fragment Shader
        ID3D11PixelShader* pixelShader = pipelineDescription.m_PixelShader == nullptr ? nullptr : static_cast<DX11_Utility::DX11_PixelShaderPackage*>(pipelineDescription.m_PixelShader->m_InternalState.get())->m_Resource.Get();
        m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->PSSetShader(pixelShader, nullptr, 0);

        // Our cubemap texture is always bounded in our buffer.
        m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->DrawIndexed(m_SkyboxEntity->GetTransform()->m_Children[0]->GetEntity()->GetComponent<Mesh>()->m_Indices.size(), 0, 0);

        return true;
    }

    bool Skybox::RenderForResources() const
    {
        const RHI_PipelineState* pipelineStateObject = &m_Renderer->m_PSO_Object_Sky;
        const RHI_PipelineState_Description& pipelineDescription = pipelineStateObject != nullptr ? pipelineStateObject->GetDescription() : RHI_PipelineState_Description();

        auto internalState = DX11_Utility::ToInternal(pipelineStateObject);

        // Bind Input Layout
        ID3D11InputLayout* inputLayout = pipelineDescription.m_InputLayout == nullptr ? nullptr : internalState->m_InputLayout.Get();
        m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->IASetInputLayout(inputLayout);

        // Bind Sky Vertex Shader
        ID3D11VertexShader* vertexShader = pipelineDescription.m_VertexShader == nullptr ? nullptr : static_cast<DX11_Utility::DX11_VertexShaderPackage*>(m_RectToCubemapVSShader.m_InternalState.get())->m_Resource.Get();
        m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->VSSetShader(vertexShader, nullptr, 0);

        // Bind Sky Fragment Shader
        ID3D11PixelShader* pixelShader = pipelineDescription.m_PixelShader == nullptr ? nullptr : static_cast<DX11_Utility::DX11_PixelShaderPackage*>(m_RectToCubemapPSShader.m_InternalState.get())->m_Resource.Get();
        m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->PSSetShader(pixelShader, nullptr, 0);

        // Our cubemap texture is always bounded in our buffer.
        m_Renderer->m_GraphicsDevice->m_DeviceContextImmediate->DrawIndexed(m_SkyboxEntity->GetTransform()->m_Children[0]->GetEntity()->GetComponent<Mesh>()->m_Indices.size(), 0, 0);

        return true;
    }
}