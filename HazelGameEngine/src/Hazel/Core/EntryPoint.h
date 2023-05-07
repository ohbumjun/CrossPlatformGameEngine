#pragma once

#ifdef HZ_PLATFORM_WINDOWS

// �ܺο� ���ǵ� �Լ��̰�, Application �� �������ش�.
extern Hazel::Application* Hazel::CreateApplication();

int main(int argc, char* argv)
{
	/*Log Init*/
	Hazel::Log::Init();

	Hazel::Log::GetCoreLogger()->warn("Initialized Log");
	// HZ_CORE_WARN("Initialized Log");
	Hazel::Log::GetClientLogger()->info("Hello");

	auto* app = Hazel::CreateApplication();
	app->Run();
	delete app;
}
#endif