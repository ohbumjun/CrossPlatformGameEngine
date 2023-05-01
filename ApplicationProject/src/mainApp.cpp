#include <Hazel.h>

// 참고 : imgui 가 include 되는 원리 (혹은 link 원리)
// imgui 를 빌드하여 static library로 만든다.
// (imgui -> static lib)
// 그리고 hazel 로 static lib 형태로 include 가 되게 한다.
// hazel 은 dll 형태로 뽑아지게 된다.
// (hazel -> dll)
// client 는 dll 형태의 hazel 을 include 하는 것이다.
#include "imgui/imgui.h"

class ExampleLayer : public Hazel::Layer
{
public: 
	ExampleLayer()
		: Layer("Example"),
		m_Camera{ -1.6f, 1.6f, -0.9f, 0.9f },
		m_CameraPos(0.f)
	{

		// Create Vertex Array
		m_VertexArray.reset(Hazel::VertexArray::Create());

		// 아래 위치를 통해 Rendering 을 하면
		// 가운데가 0,0,0 이 된다.
		/*
		layout 에 a_Position 만 있을 경우
		float vertices[3 * 3] = {
			-0.5f, -0.5f, 0.0f, // one point
			0.5f,  -0.5f, 0.f,
			0.0f, 0.5f, 0.0f
		};
		*/
		float vertices[3 * 7] = {
			/*pos*/-0.5f, -0.5f, 0.0f,  /*color*/ 1.0f, 0.0f, 1.0f, 1.0f,
			0.5f,  -0.5f, 0.f,			/*color*/1.0f, 0.0f, 1.0f, 1.0f,
			0.0f, 0.5f, 0.0f,			/*color*/1.0f, 0.0f, 1.0f, 1.0f
		};

		std::shared_ptr<Hazel::VertexBuffer> vertexBuffer;
		vertexBuffer.reset(Hazel::VertexBuffer::Create(vertices, sizeof(vertices)));

		Hazel::BufferLayout layout = {
			{Hazel::ShaderDataType::Float3, "a_Position"},
			{Hazel::ShaderDataType::Float4, "a_Color"}
		};

		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);

		uint32_t indices[3] = { 0, 1, 2 };
		std::shared_ptr<Hazel::IndexBuffer> indexBuffer;
		indexBuffer.reset(Hazel::IndexBuffer::Create(indices, 3));
		m_VertexArray->SetIndexBuffer(indexBuffer);

		/*Square*/
		m_SquareArray.reset(Hazel::VertexArray::Create());

		float squareVertices[3 * 4] = {
			-0.75f, -0.5f, 0.0f,
			0.75f,  -0.5f, 0.f,
			0.75f,  0.5f, 0.0f,
			-0.75f, 0.5f, 0.0f
		};

		std::shared_ptr<Hazel::VertexBuffer> squareVB;
		squareVB.reset(Hazel::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));

		Hazel::BufferLayout squareVBLayout = {
			{Hazel::ShaderDataType::Float3, "a_Position"}
		};

		squareVB->SetLayout(squareVBLayout);
		m_SquareArray->AddVertexBuffer(squareVB);

		uint32_t squareIndices[] = { 0, 1, 2, 2, 3, 0 };
		std::shared_ptr<Hazel::IndexBuffer> squareIdxB;
		squareIdxB.reset(Hazel::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		m_SquareArray->SetIndexBuffer(squareIdxB);

		std::string vertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			uniform mat4 u_ViewProjection;

			out vec3 v_Position;
			out vec4 v_Color;

			void main()
			{
				v_Position = a_Position;
				v_Color    = a_Color;
				gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
			}
		)";

		std::string fragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			in vec4 v_Color;

			void main()
			{
				// color = vec4(v_Position * 0.5f + 0.5f, 1.0);
				color = v_Color;
			}
		)";

		m_Shader.reset(new Hazel::Shader(vertexSrc, fragmentSrc));

		std::string vertexSrc2 = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;

			uniform mat4 u_ViewProjection;

			out vec3 v_Position;

			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * vec4(a_Position, 1.0);	
			}
		)";

		std::string fragmentSrc2 = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec3 v_Position;

			void main()
			{
				color = vec4(0.2f, 0.2, 0.8f, 1.0);
			}
		)";

		m_BlueShader.reset(new Hazel::Shader(vertexSrc2, fragmentSrc2));
	}

	void OnUpdate(Hazel::Timestep ts) override
	{
		/*Pos*/
		if (Hazel::Input::IsKeyPressed(HZ_KEY_LEFT))
		{
			m_CameraPos.x -= m_CameraMoveSpeed * ts;
		}
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_RIGHT))
		{
			m_CameraPos.x += m_CameraMoveSpeed * ts;
		}
		
		if (Hazel::Input::IsKeyPressed(HZ_KEY_UP))
		{
			m_CameraPos.y += m_CameraMoveSpeed * ts;
		}
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_DOWN))
		{
			m_CameraPos.y -= m_CameraMoveSpeed * ts;
		}

		/*Rot*/
		if (Hazel::Input::IsKeyPressed(HZ_KEY_A))
		{
			m_CameraRot += m_CameraRotSpeed;
		}
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_D))
		{
			m_CameraRot -= m_CameraRotSpeed;
		}

		Hazel::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.f });
		Hazel::RenderCommand::Clear();

		m_Camera.SetPosition(m_CameraPos);
		m_Camera.SetRotation(m_CameraRot);

		// Renderer::BeginScene(camera, lights, environment);
		// Scene 을 그리기 위해 필요한 모든 것을 한번에 그려낸다.
		Hazel::Renderer::BeginScene(m_Camera);

		Hazel::Renderer::Submit(m_SquareArray, m_BlueShader);
		Hazel::Renderer::Submit(m_VertexArray, m_Shader);

		Hazel::Renderer::EndScene();
	}

	void OnEvent(Hazel::Event& event) override
	{
		if (event.GetEventType() == Hazel::EventType::KeyPressed)
		{
			Hazel::KeyPressedEvent& e = (Hazel::KeyPressedEvent&)event;
			if (e.GetKeyCode() == HZ_KEY_TAB)
				HZ_TRACE("Tab key is pressed (event)!");
			HZ_TRACE("{0}", (char)e.GetKeyCode());
		}
	}

private :
	std::shared_ptr<Hazel::Shader> m_Shader;
	std::shared_ptr<Hazel::VertexArray> m_VertexArray;

	std::shared_ptr<Hazel::Shader> m_BlueShader;
	std::shared_ptr<Hazel::VertexArray> m_SquareArray;

	Hazel::OrthographicCamera m_Camera;
	glm::vec3 m_CameraPos;
	float m_CameraMoveSpeed = 0.1f;
	float m_CameraRotSpeed = 0.1f;
	float m_CameraRot     = 0.0f;
};

class Sandbox : public Hazel::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
		
		// Client 측에서 ImGuiLayer 를 세팅하게 해주고 싶지 않다.
		// Engine 측 Application 에서 추가하게 할 것이다.
		// PushOverlay(new Hazel::ImGuiLayer());


	}

	~Sandbox()
	{

	}

};

Hazel::Application* Hazel::CreateApplication()
{
	return new Sandbox();
}