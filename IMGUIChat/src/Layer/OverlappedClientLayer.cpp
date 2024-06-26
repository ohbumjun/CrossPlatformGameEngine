#include "OverlappedClientLayer.h"
#include "Hazel/Core/Application/Application.h"
#include "Hazel/Core/ID/UUID.h"
#include "ServerInfo.h"
#include "Util/Util.h"

OverlappedClientLayer::~OverlappedClientLayer()
{
}

void OverlappedClientLayer::OnAttach()
{
    initialize();

    m_ReceiveThread.SetThreadName(const_cast<char *>("ReceiveThread"));

    // Thread 가, Worker의 Execute 함수를 실행할 수 있게 한다.
    m_ReceiveThread.StartThread([&]() { this->receiveMessage(); });

    m_CricSect = Hazel::ThreadUtils::CreateCritSect();
}

void OverlappedClientLayer::OnDetach()
{
    WSACloseEvent(evObj);
    closesocket(hSocket);
    WSACleanup();
}

void OverlappedClientLayer::OnUpdate(Hazel::Timestep ts)
{
    Hazel::RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1.f});
    Hazel::RenderCommand::Clear();
}

void OverlappedClientLayer::OnEvent(Hazel::Event &event)
{
}

void OverlappedClientLayer::OnImGuiRender()
{
    ImGuiChatWindow();
}
 
void OverlappedClientLayer::ImGuiChatWindow()
{
    ImGui::Begin("Chat", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    char messageBuffer[256];
    memset(messageBuffer, 0, sizeof(messageBuffer));
    strcpy_s(messageBuffer, sizeof(messageBuffer), inputText.c_str());

    if (ImGui::InputText("##Name", messageBuffer, sizeof(messageBuffer)))
    {
        inputText = messageBuffer;
    }

    // Send button
    if (ImGui::Button("Send", ImVec2(100, 0)))
    {
        if (inputText.length() > 0)
        {
            int sendBytes = 0;
            char msg[BUF_SIZE];

            memcpy(msg, inputText.c_str(), inputText.length() + 1);

            WSABUF dataBuf;
            dataBuf.len = strlen(msg) ;
            dataBuf.buf = msg;

            // WSASend 함수를 호출하고나서도 여전히 IO가 진행중이라면, 즉, 데이터를 계속해서 전송중이라면
            // 만약 리턴값이 SOCKET_ERROR 가 아니라면,
            // 해당 함수 호출과 동시에 데이터 전송이 완료된 것, 그리고 sendBytes 에는 전송한 데이터의 크기가 들어있다.
            // IO 가 완료되면 WSAOverlapped 구조체 변수가 참조하는 Event 오브젝트는 signaled 상태가 된다.
            if (WSASend(hSocket,
                        &dataBuf,
                        1,
                        (LPDWORD)&sendBytes,
                        0,
                        &overlapped,
                        NULL) == SOCKET_ERROR)
            {
                // WSASend 함수를 호출하고나서도 여전히 IO가 진행중이라면, 즉, 데이터를 계속해서 전송중이라면
                if (WSAGetLastError() == WSA_IO_PENDING)
                {
                    // 해당 IO가 끝나면 overlapped.hEvent, 즉 evObj 이벤트 커널 오브젝트가 signaled 상태가 된다
                    // signaled 상태가 될 때까지 기다린다. (WSA_INFINITE)
                    WSAWaitForMultipleEvents(1,
                                             &evObj,
                                             TRUE,
                                             WSA_INFINITE,
                                             FALSE);

                    // 실제 전송된 데이터의 크기를 확인
                    // 4번째 : fwait -> 여전히 io 가 진행중인 상황인 경우, true 전달시 io 가 완료될 때까지
                    //							대기하게 되고 false 를 전달시 FALSE 반환하면서 함수 빠져나온다.
                    //	5번째 : oob 메시지 등 부가 정보를 확인하기 위한 정보
                    WSAGetOverlappedResult(hSocket,
                                           &overlapped,
                                           (LPDWORD)&sendBytes,
                                           FALSE,
                                           NULL);

                   bool h = true;
                }
                else
                {
                    NetworkUtil::ErrorHandling("WSASend() Error");
                }
            }
        }
    }

    ImGui::End();
}

void OverlappedClientLayer::initialize()
{
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        NetworkUtil::ErrorHandling("WSAStartUp() Error");

    // WSA_FLAG_OVERLAPPED : overlapped io 가 가능한 소켓의 생성
    hSocket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

    memset(&sendAddr, 0, sizeof(sendAddr));
    sendAddr.sin_family = AF_INET;
    sendAddr.sin_addr.s_addr = inet_addr(TEST_SERVER_IP_ADDRESS);
    sendAddr.sin_port = htons(atoi(TEST_SERVER_PORT));

    if (connect(hSocket, (SOCKADDR *)&sendAddr, sizeof(sendAddr)) ==
        SOCKET_ERROR)
        NetworkUtil::ErrorHandling("connect() Error");

    // manual-reset, non-signaled 형태의 이벤트 생성
    evObj = WSACreateEvent();

    // 모든 비트 0으로 초기화
    memset(&overlapped, 0, sizeof(overlapped));

    // 해당 overlapped 구조체 변수의 event 오브젝트를 사용하여 데이터의 전송 등이
    // 완료되었는지 등을 확인한다.
    overlapped.hEvent = evObj;
}

void OverlappedClientLayer::receiveMessage()
{
    static char recvBuffer[BUF_SIZE];
    int recvBytes = 0, flags = 0;
    char ReceivedIP[46] = {0};

    while (1)
    {
        WSABUF dataBuf;
        dataBuf.len = BUF_SIZE;
        dataBuf.buf = recvBuffer;

        if (WSARecv(hSocket,
            &dataBuf,
            1,
            (LPDWORD)&recvBytes,
            (LPDWORD)&flags,
            &overlapped,
            NULL) == SOCKET_ERROR)
        {
            // 해당 함수 호출 이후에도, 데이터의 수신이 계속된다면
            if (WSAGetLastError() == WSA_IO_PENDING)
            {
                // 해당 IO가 끝나면 overlapped.hEvent,
                // 즉 evObj 이벤트 커널 오브젝트가 signaled 상태가 된다
                // signaled 상태가 될 때까지 기다린다. (WSA_INFINITE)
                // ex) Client 측에서 송신이 끝나면, 이제 Signaled 상태기 된다는 것이다.
                WSAWaitForMultipleEvents(1, &evObj, TRUE, WSA_INFINITE, FALSE);

                // 실제 전송된 데이터의 크기를 확인
                WSAGetOverlappedResult(hSocket,
                                       &overlapped,
                                       (LPDWORD)&recvBytes,
                                       FALSE,
                                       NULL);

                sockaddr_in ClientAddr;
                inet_ntop(AF_INET, &ClientAddr.sin_addr, (PSTR)ReceivedIP, 46);
                std::cout << "Received from: " << ReceivedIP << ", "
                          << ntohs(ClientAddr.sin_port) << "\n";

                Hazel::ThreadUtils::LockCritSect(m_CricSect);

                m_ReceivedMessage.push_back(recvBuffer);

                Hazel::ThreadUtils::UnlockCritSect(m_CricSect);
            }
            else
            {
                NetworkUtil::ErrorHandling("WSARecv() Error");
            }
        }
    }
}
