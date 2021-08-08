#include "Aurora.h"
#include "Sprite.h"
#include "../Renderer/Renderer.h"

namespace Aurora
{
    bool Sprite::Initialize(EngineContext* engineContext, float width, float height, std::string spritePath, CB_VS_VertexShader_2D vertexShader)
    {
        m_EngineContext = engineContext;
        m_Texture = m_EngineContext->GetSubsystem<ResourceCache>()->LoadTexture("../Resources/Textures/Gizmos/Light.png", "Light");
        m_VertexShaderCB = &vertexShader;

        std::vector<Vertex2D> vertexData =
        {
            Vertex2D(-0.5f, -0.5f, 0.0f, 0.0f, 0.0f), //TopLeft
            Vertex2D(0.5f, -0.5f, 0.0f, 1.0f, 0.0f), //TopRight
            Vertex2D(-0.5, 0.5, 0.0f, 0.0f, 1.0f), //Bottom Left
            Vertex2D(0.5, 0.5, 0.0f, 1.0f, 1.0f), //Bottom Right
        };

        std::vector<DWORD> indexData =
        {
            0, 1, 2,
            2, 1, 3
        };

        HRESULT hr = m_Vertices.Initialize(m_EngineContext->GetSubsystem<Renderer>()->m_GraphicsDevice->m_Device.Get(), vertexData.data(), vertexData.size());
        // COM_ERROR_IF_FAILED(hr, "Failed to initialize vertex buffer for sprite.");

        hr = m_IndexBuffer.Initialize(m_EngineContext->GetSubsystem<Renderer>()->m_GraphicsDevice->m_Device.Get(), indexData.data(), indexData.size());
        // COM_ERROR_IF_FAILED(hr, "Failed to initialize index buffer for mesh.");

        return false;
    }

    void Sprite::Draw(XMMATRIX orthographicMatrix)
    {
        XMMATRIX wvpMatrix = m_WorldMatrix * orthographicMatrix;
  

        m_EngineContext->GetSubsystem<Renderer>()->m_GraphicsDevice->m_DeviceContextImmediate->PSSetShaderResources(0, 1, texture->GetTextureResourceViewAddress());

        const UINT offsets = 0;
        m_EngineContext->GetSubsystem<Renderer>()->m_GraphicsDevice->m_DeviceContextImmediate->IASetVertexBuffers(0, 1, m_Vertices.GetAddressOf(), m_Vertices.StridePtr(), &offsets);
        m_EngineContext->GetSubsystem<Renderer>()->m_GraphicsDevice->m_DeviceContextImmediate->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
        m_EngineContext->GetSubsystem<Renderer>()->m_GraphicsDevice->m_DeviceContextImmediate->DrawIndexed(m_IndexBuffer.IndexCount(), 0, 0);
    }

    float Sprite::GetWidth()
    {
        return 0.0f;
    }

    float Sprite::GetHeight()
    {
        return 0.0f;
    }
}