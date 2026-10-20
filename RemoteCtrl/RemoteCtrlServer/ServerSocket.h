#pragma once
class CServerSocket
{
public:
	static CServerSocket* getInstance() {
		if (m_instance == NULL) {
			m_instance = new CServerSocket();
		}
		return m_instance;
	}
	bool InitSocket() {
		m_sock = socket(AF_INET, SOCK_STREAM, 0);
		sockaddr_in m_sock_addr;
		m_sock_addr.sin_family = AF_INET;
		m_sock_addr.sin_port = htons(6657);
		m_sock_addr.sin_addr.s_addr = INADDR_ANY;
		if (bind(m_sock, (sockaddr*)&m_sock_addr, 0) == -1) {
			return false;
		}
		if (listen(m_sock, 1) == -1) {
			return false;
		}
		return true;
	}

	bool AcceptCLient() {
		sockaddr_in client_addr;
		int cli_sz = sizeof(client_addr);
		m_client = accept(m_sock, (sockaddr*)&client_addr, &cli_sz);
		if (m_client == -1) {
			return false;
		}
		return true;
	}

	int DealCommand() {
		char buffer[1024] = "";
		while (true) {
			int len = recv(m_client, buffer, sizeof(buffer), 0);
			if (len <= 0) {
				return -1;
			}
			//TODO：处理数据
		}
	}
private:
	SOCKET m_sock;
	SOCKET m_client;
	//拷贝赋值构造不可调用
	CServerSocket(const CServerSocket&) = delete;
	CServerSocket& operator=(const CServerSocket&) = delete;
	CServerSocket()
	{
		if (!InitSockEnv()) {
			MessageBox(NULL, _T("无法初始化套接字环境，请检查网络设置！"), _T("初始化错误！"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_sock = SOCKET_ERROR;
		m_client = SOCKET_ERROR;
	}
	~CServerSocket()
	{
		WSACleanup();
	}
	bool InitSockEnv()
	{
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data)!=0) {
			return false;
		}
		return true;
	 }
	static void releaseInstance() {
		CServerSocket* tmp;
		tmp = m_instance;
		m_instance = nullptr;
		delete tmp;
	}

	static CServerSocket* m_instance;

	class CHelper
	{
	public:
		CHelper() {
			CServerSocket::getInstance();
		}
		~CHelper() {
			CServerSocket::releaseInstance();
		}

	private:

	};
	static CHelper m_helper;
};
