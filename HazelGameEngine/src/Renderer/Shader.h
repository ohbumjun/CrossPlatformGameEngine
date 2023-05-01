#pragma once

#include <string>
#include <glm/glm.hpp>

namespace Hazel
{
	class Shader
	{
	public :
		Shader(const std::string& vertexSrc, const std::string& fragmentSrc);
		~Shader();

		void Bind() const;
		void Unbind() const;

		void UploadUniformFloat4(const std::string& name, const glm::vec4& vec);
		void UploadUniformMat4(const std::string& name, const glm::mat4& mat);

	private :
		// 현재 해당 객체를 식벽하는 ID
		uint32_t m_RendererID;
	};
}