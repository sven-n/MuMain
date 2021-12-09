#ifndef _WSCTLC_ADDON_H_
#define _WSCTLC_ADDON_H_

// - 이현

#define MAX_SENDBUF		8192
#define MAX_RECVBUF		8192

class CPacket{
	BYTE	m_byBuffer[MAX_RECVBUF];
	int		m_size;

public:
	CPacket(BYTE* byBuf, int size) {
		memcpy(m_byBuffer, byBuf, size);
		m_byBuffer[size] = 0xFD;	//. 메모리 블럭표시
		m_size = size;
	}
	~CPacket(){}

	int GetSize() const { return m_size; }
	BYTE* GetBuffer() { return m_byBuffer; }
};

class CPacketQueue
{
	class CGarbageCollection
	{
		std::list<CPacket*>		m_listGarbage;
		bool	m_bCheckIntegrity;
	public:
		CGarbageCollection() : m_bCheckIntegrity(false){}
		~CGarbageCollection(){ Clear(); }

		void Registry(CPacket* pPacket){
			m_listGarbage.push_back(pPacket);
		}
		void Unregistry(CPacket* pPacket){
			m_listGarbage.remove(pPacket);
		}
		
		void EnableCheckIntegrity() { m_bCheckIntegrity = true; }
		void DisableCheckIntegrity() { m_bCheckIntegrity = false; }

		void Clear(){
			if(!m_listGarbage.empty()){
				std::list<CPacket*>::iterator li = m_listGarbage.begin();
				for(; li != m_listGarbage.end(); li++){
#ifdef _DEBUG
					if(m_bCheckIntegrity){	//. 메모리 무결성 검사
						BYTE MemBlock = *((*li)->GetBuffer()+(*li)->GetSize());
						assert(MemBlock == 0xFD);
					}
#endif // _DEBUG
					delete (*li);
				}
				m_listGarbage.clear();
			}
		}
	};

	std::queue<CPacket*>	m_queuePacket;
	CGarbageCollection*		m_pGabageCollection;

public:
	CPacketQueue(){
		m_pGabageCollection = new CGarbageCollection;
#ifdef _DEBUG
		m_pGabageCollection->EnableCheckIntegrity();
#endif // _DEBUG
	}
	~CPacketQueue(){
		ClearGarbage();
		
		while(!m_queuePacket.empty()){
			delete m_queuePacket.front();
			m_queuePacket.pop();
		}

		delete m_pGabageCollection;
	}

	void PushPacket(BYTE* byBuf, int size) {
		m_queuePacket.push(new CPacket(byBuf, size));
	}
	void PopPacket(){
		m_pGabageCollection->Registry(m_queuePacket.front());
		m_queuePacket.pop();
	}
	bool IsEmpty() const { return m_queuePacket.empty(); }
	size_t GetQueueSize() const { return m_queuePacket.size(); }


	CPacket* FrontPacket() const {
		return m_queuePacket.front();
	}

	void ClearGarbage() { m_pGabageCollection->Clear(); }
};

#endif // _WSCTLC_ADDON_H_