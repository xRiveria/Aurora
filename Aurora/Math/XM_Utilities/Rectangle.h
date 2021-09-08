#pragma once
#include "../Graphics/DX11_Refactored/DX11_VertexBuffer.h"
#include "../Graphics/DX11_Refactored/DX11_IndexBuffer.h"

namespace Aurora
{
    class Renderer;
}

namespace Aurora::Math
{
    class Rectangle
    {
    public:
        Rectangle()
        {
            left = 0.0f;
            top = 0.0f;
            right = 0.0f;
            bottom = 0.0f;
        }

        Rectangle(const float left, const float top, const float right, const float bottom)
        {
            this->left = left;
            this->top = top;
            this->right = right;
            this->bottom = bottom;
        }

        Rectangle(const Rectangle& rectangle)
        {
            left = rectangle.left;
            top = rectangle.top;
            right = rectangle.right;
            bottom = rectangle.bottom;
        }

        ~Rectangle() = default;

        bool operator==(const Rectangle & rhs) const
        {
            return
                left == rhs.left &&
                top == rhs.top &&
                right == rhs.right &&
                bottom == rhs.bottom;
        }

        bool operator!=(const Rectangle & rhs) const
        {
            return
                left != rhs.left ||
                top != rhs.top ||
                right != rhs.right ||
                bottom != rhs.bottom;
        }

        bool IsDefined() const
        {
            return  left != 0.0f ||
                top != 0.0f ||
                right != 0.0f ||
                bottom != 0.0f;
        }

        float Width()  const { return right - left; }
        float Height() const { return bottom - top; }
        
        bool CreateBuffers(Renderer* renderer);

        static int GetIndexCount() { return 6; }
        DX11_VertexBuffer* GetVertexBuffer() const { return m_VertexBuffer.get(); }
        DX11_IndexBuffer* GetIndexBuffer() const { return m_IndexBuffer.get(); }
        
    public:
        float left = 0.0f;
        float top = 0.0f;
        float right = 0.0f;
        float bottom = 0.0f;

    private:
        std::shared_ptr<DX11_VertexBuffer> m_VertexBuffer;
        std::shared_ptr<DX11_IndexBuffer> m_IndexBuffer;
    };
}