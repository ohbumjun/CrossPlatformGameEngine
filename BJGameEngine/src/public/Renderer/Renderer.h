#pragma once

#include "Camera/OrthographicCamera.h"
#include "Hazel/Resource/Shader/Shader.h"
#include "RenderCommand.h"
#include "Renderer/Buffer/VertexArray.h"
#include "RendererAPI.h"

namespace Hazel
{
class HAZEL_API Renderer
{
public:
    static void BeginScene(OrthographicCamera &camera);
    static void EndScene();
    static void Init();
    static void ShutDown();
    static void OnWindowResize(uint32_t width, uint32_t height);
    static void Submit(const std::shared_ptr<VertexArray> &vertexArray,
                       const std::shared_ptr<Shader> &shader,
                       const glm::mat4 &transform = glm::mat4(1.f));

    inline static RendererAPI::API GetAPI()
    {
        return RendererAPI::GetAPI();
    }

private:
    struct SceneData
    {
        glm::mat4 ViewProjectionMatrix;
    };

    static SceneData *m_SceneData;
};
} // namespace Hazel
