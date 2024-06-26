#include <Hazel.h>

/*Entry Point -> Application 프로젝트 한 곳에서만 include 해야 한다.
따라서main app 이 있는 곳으로 include 처리한다.
*/
#include "Hazel/Core/EntryPoint.h"
#include "Layer/ChatServerLayer.h"
#include "Layer/EchoTCPServerLayer.h"
#include "Layer/EchoTCPClientLayer.h"
#include "Layer/MultiCastSenderLayer.h"
#include "Layer/MultiCastReceiverLayer.h"
#include "Layer/OverlappedServerLayer.h"
#include "Layer/OverlappedClientLayer.h"
#include "ServerInfo.h"

// temp 변수
std::map<std::string, NetworkType> stringToEnum = {
    {"CLIENT", NetworkType::CLIENT},
    {"SERVER", NetworkType::SERVER},

    {"ECO_TCP_CLIENT", NetworkType::ECO_TCP_CLIENT},
    {"ECO_TCP_CLIENT", NetworkType::ECO_TCP_SERVER},

    {"MULTICAST_RECEIVER", NetworkType::MULTICAST_RECEIVER},
    {"MULTICAST_SENDER", NetworkType::MULTICAST_SENDER},

    {"OVERLAPPED_RECEIVER", NetworkType::OVERLAPPED_RECEIVER},
    {"OVERLAPPED_SENDER", NetworkType::OVERLAPPED_SENDER},

    {"TCP_CLIENT", NetworkType::TCP_CLIENT},
    {"TCP_SERVER", NetworkType::TCP_SERVER}
};

class TCPServerApp : public Hazel::Application
{
public:
    TCPServerApp(const Hazel::ApplicationSpecification &specification)
        : Hazel::Application(specification)
    {
        // PushLayer(new ChatServerLayer());
        PushLayer(new OverlappedClientLayer());
    }

    ~TCPServerApp()
    {
    }
};

class TCPClientApp : public Hazel::Application
{
public:
    TCPClientApp(const Hazel::ApplicationSpecification &specification)
        : Hazel::Application(specification)
    {
        // PushLayer(new ChatServerLayer());
        PushLayer(new OverlappedClientLayer());
    }

    ~TCPClientApp()
    {
    }
};

class OverlappedSenderApp : public Hazel::Application
{
public:
    OverlappedSenderApp(const Hazel::ApplicationSpecification &specification)
        : Hazel::Application(specification)
    {
        // PushLayer(new ChatServerLayer());
        PushLayer(new OverlappedClientLayer());
    }

    ~OverlappedSenderApp()
    {
    }
};


class OverlappedReceiverApp : public Hazel::Application
{
public:
    OverlappedReceiverApp(const Hazel::ApplicationSpecification &specification)
        : Hazel::Application(specification)
    {
        // PushLayer(new ChatServerLayer());
        PushLayer(new OverlappedServerLayer());
    }

    ~OverlappedReceiverApp()
    {
    }
};

class EchoTCPClientApp : public Hazel::Application
{
public:
    EchoTCPClientApp(const Hazel::ApplicationSpecification &specification)
        : Hazel::Application(specification)
    {
        // PushLayer(new ChatServerLayer());
        PushLayer(new EchoTCPClientLayer());
    }

    ~EchoTCPClientApp()
    {
    }
};

class EchoTCPServerApp : public Hazel::Application
{
public:
    EchoTCPServerApp(const Hazel::ApplicationSpecification &specification)
        : Hazel::Application(specification)
    {
        // PushLayer(new ChatServerLayer());
        PushLayer(new EchoTCPServerLayer());
    }

    ~EchoTCPServerApp()
    {
    }
};

class MulticastReceiverApp : public Hazel::Application
{
public:
    MulticastReceiverApp(const Hazel::ApplicationSpecification &specification)
        : Hazel::Application(specification)
    {
        // PushLayer(new ChatServerLayer());
        PushLayer(new MultiCastReceiverLayer());
    }

    ~MulticastReceiverApp()
    {
    }
};

class MulticastSenderApp : public Hazel::Application
{
public:
    MulticastSenderApp(const Hazel::ApplicationSpecification &specification)
        : Hazel::Application(specification)
    {
        // PushLayer(new ChatServerLayer());
        PushLayer(new MultiCastSenderLayer());
    }

    ~MulticastSenderApp()
    {
    }
};

Hazel::Application *Hazel::CreateApplication(
    Hazel::ApplicationCommandLineArgs args)
{
    Hazel::ApplicationSpecification spec;
    spec.Name = "IMGUIChat";
    spec.WorkingDirectory = "";
    spec.CommandLineArgs = args;

    NetworkType netType;

    if (args.GetCount() > 1)
    {
        std::string netTypeStr = args[1];

        if (stringToEnum.find(netTypeStr) != stringToEnum.end())
        {
            netType = stringToEnum[netTypeStr];
        }
        else
        {
            HZ_CORE_ASSERT(false, "Wrong Network Type");
        }
	}
    else
    {
        // netType = NetworkType::ECO_TCP_SERVER;
        netType = NetworkType::OVERLAPPED_RECEIVER;
    }
    
    switch (netType)
    {
    case NetworkType::SERVER:
        break;
    case NetworkType::CLIENT:
        break;
    case NetworkType::ECO_TCP_SERVER:
        return new EchoTCPServerApp(spec);
    case NetworkType::ECO_TCP_CLIENT:
        return new EchoTCPClientApp(spec);
    case NetworkType::MULTICAST_RECEIVER:
        return new MulticastReceiverApp(spec);
    case NetworkType::MULTICAST_SENDER:
        return new MulticastSenderApp(spec);
    case NetworkType::OVERLAPPED_RECEIVER:
        return new OverlappedReceiverApp(spec);
    case NetworkType::OVERLAPPED_SENDER:
        return new OverlappedSenderApp(spec);
    default:
        break;
    }

}