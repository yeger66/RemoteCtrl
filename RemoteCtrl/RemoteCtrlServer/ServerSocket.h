#pragma once
#include "pch.h"
#include "framework.h"

class CPacket
{
public:
	CPacket():sHead(0),nLength(0), sCmd(0),strData(""), sSum(0) {}
	CPacket(const CPacket& pack) {
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}
	CPacket& operator=(const CPacket& pack) {
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
		return *this;
	}
	CPacket(const BYTE* pData,size_t nSize) {
		//数据不够 构造失败先按return处理 

		//判断不含包数据长度够不够解析一次包
		if (nSize < 2 + 4 + 2 + 2) {
			return;
		}

		size_t i = 0;
		//找到包头
		for (; i < nSize; i++){
			if (*(WORD*)(pData + i) == 0XFEFF) {
				sHead = *(WORD*)(pData + i);
				i += 2;
				break;
			}
		}
		//解析出长度数据
		nLength = *(DWORD*)(pData + i);
		i += 4;
		//通过长度来判断是否够一次包解析
		if (nLength+i > nSize ) {
			return;
		}

		//解析控制命令数据
		sCmd = *(WORD*)(pData + i);
		i += 2;
		//解析包数据
		if (nLength > 4) {
			strData.resize(nLength - 2 - 2);
			memcpy((void*)strData.c_str(), (void*)(pData + i), nLength - 4);
			i += nLength - 4;
		}
		//解析和校验数据
		sSum = *(WORD*)(pData + i);
		i += 2;
		//验证和校验
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++) {
			sum += BYTE(strData[i]) & 0XFF;
		}
		
		if (sum = sSum) {
			//TODO: nSize为局部赋值 目前属于无效动作
			nSize = i;
			return;
		}
		nSize = 0;
	}
	~CPacket() {}

public:
	//包头2+包长4+控制命令2+包数据+和校验2
	WORD sHead; //包头 固定为 0xFEFF
	DWORD nLength; //包长 （从命令开始到校验和结束）
	WORD sCmd; //控制命令（数据类型） 
	std::string strData; //包数据
	WORD sSum; //和校验
};

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
#define BUFF_SIZE 4096
	int DealCommand() {
		//char buffer[BUFFSIZE] = "";
		//while (true) {
		//	int len = recv(m_client, buffer, sizeof(buffer), 0);
		//	if (len <= 0) {
		//		return -1;
		//	}
		//	//TODO：处理数据
		//}
		//客户端socket异常 直接return
		if (m_client == -1) {
			return -1;
		}
		char* buffer = new char[BUFF_SIZE];
		memset(buffer, 0, BUFF_SIZE);
		size_t index = 0;
		while (true) {
			size_t len = recv(m_client, buffer + index, BUFF_SIZE - index, 0);
			if (len <= 0) {
				return -1;
			}
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer, len);
			if (len > 0) {
				memmove(buffer, buffer + len, BUFF_SIZE - len);
				index -= len;
				return m_packet.sCmd;
			}
		}
		return -1;
	}
private:
	SOCKET m_sock;
	SOCKET m_client;
	CPacket m_packet;
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
