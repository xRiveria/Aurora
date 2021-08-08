#pragma once
#include "../Graphics/RHI_Implementation.h"
#include <DirectXMath.h>
#include "../Core/EngineContext.h"
#include "../Resource/ResourceCache.h"

using namespace DirectX;

namespace Aurora
{
	template<class T>
	class VertexBuffer
	{
	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
		UINT stride = sizeof(T);
		UINT vertexCount = 0;

	public:
		VertexBuffer() {}

		VertexBuffer(const VertexBuffer<T>& rhs)
		{
			this->buffer = rhs.buffer;
			this->vertexCount = rhs.vertexCount;
			this->stride = rhs.stride;
		}

		VertexBuffer<T>& operator=(const VertexBuffer<T>& a)
		{
			this->buffer = a.buffer;
			this->vertexCount = a.vertexCount;
			this->stride = a.stride;
			return *this;
		}

		ID3D11Buffer* Get()const
		{
			return buffer.Get();
		}

		ID3D11Buffer* const* GetAddressOf()const
		{
			return buffer.GetAddressOf();
		}

		UINT VertexCount() const
		{
			return this->vertexCount;
		}

		const UINT Stride() const
		{
			return this->stride;
		}

		const UINT* StridePtr() const
		{
			return &this->stride;
		}

		HRESULT Initialize(ID3D11Device* device, T* data, UINT vertexCount)
		{
			if (buffer.Get() != nullptr)
				buffer.Reset();

			this->vertexCount = vertexCount;

			D3D11_BUFFER_DESC vertexBufferDesc;
			ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

			vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
			vertexBufferDesc.ByteWidth = stride * vertexCount;
			vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			vertexBufferDesc.CPUAccessFlags = 0;
			vertexBufferDesc.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA vertexBufferData;
			ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
			vertexBufferData.pSysMem = data;

			HRESULT hr = device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, this->buffer.GetAddressOf());
			return hr;
		}
	};

	class IndexBuffer
	{
	private:
		IndexBuffer(const IndexBuffer& rhs);

	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
		UINT indexCount = 0;
	public:
		IndexBuffer() {}

		ID3D11Buffer* Get()const
		{
			return buffer.Get();
		}

		ID3D11Buffer* const* GetAddressOf()const
		{
			return buffer.GetAddressOf();
		}

		UINT IndexCount() const
		{
			return this->indexCount;
		}

		HRESULT Initialize(ID3D11Device* device, DWORD* data, UINT indexCount)
		{
			if (buffer.Get() != nullptr)
				buffer.Reset();

			this->indexCount = indexCount;
			//Load Index Data
			D3D11_BUFFER_DESC indexBufferDesc;
			ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
			indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
			indexBufferDesc.ByteWidth = sizeof(DWORD) * indexCount;
			indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			indexBufferDesc.CPUAccessFlags = 0;
			indexBufferDesc.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA indexBufferData;
			indexBufferData.pSysMem = data;
			HRESULT hr = device->CreateBuffer(&indexBufferDesc, &indexBufferData, buffer.GetAddressOf());
			return hr;
		}
	};

    struct Vertex2D
    {
        Vertex2D() {}
        Vertex2D(float x, float y, float z, float u, float v) : m_Position(x, y, z), m_TexCoord(u, v)
        {

        }

        XMFLOAT3 m_Position;
        XMFLOAT2 m_TexCoord;
    };

    struct CB_VS_VertexShader_2D
    {
        XMMATRIX m_MVPMatrix;
    };

    class Sprite
    {
        bool Initialize(EngineContext* engineContext, float width, float height, std::string spritePath, CB_VS_VertexShader_2D vertexShader);
        void Draw(XMMATRIX orthographicMatrix);
        float GetWidth();
        float GetHeight();
            
    private:
        CB_VS_VertexShader_2D* m_VertexShaderCB = nullptr;
        XMMATRIX m_WorldMatrix = XMMatrixIdentity();
        std::shared_ptr<AuroraResource> m_Texture;
        
		IndexBuffer m_IndexBuffer;
		VertexBuffer<Vertex2D> m_Vertices;

        EngineContext* m_EngineContext;
    };
}