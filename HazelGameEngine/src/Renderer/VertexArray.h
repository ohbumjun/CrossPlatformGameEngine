#pragma once

#include "Renderer/Buffer.h"

namespace Hazel
{
	class VertexArray
	{
	public :
		virtual ~VertexArray() {};

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
	
		virtual void AddVertexBuffer(std::shared_ptr<VertexBuffer>& buffer)= 0;
		virtual void SetIndexBuffer(std::shared_ptr<IndexBuffer>& buffer)= 0;
	
		virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffer() const = 0;
		virtual const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const = 0;

		static VertexArray* Create();
	};


}

