#pragma once
class CServerSocket
{
public:
	CServerSocket()
	{
		if (!InitSockEnv()) {
			MessageBox(NULL, _T("无法初始化套接字环境，请检查网络设置！"), _T("初始化错误！"), MB_OK | MB_ICONERROR);
			exit(0);
		}
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
};

extern CServerSocket server;