
#define MAX_CHUNK_LENGTH 128

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

//

#define m_TX_s_RX_Pipe "m_tx_s_rx_uart.fifo"

#define s_TX_m_RX_Pipe "s_tx_m_rx_uart.fifo"

//

int msleep(long msec);

struct chunkNode
{
	int length;
	char buffer[MAX_CHUNK_LENGTH];
	chunkNode* pNext;
};

// Credit Ref: https://www.techiedelight.com/queue-implementation-cpp/
class chunkQueue
{
	protected:
		chunkNode* pLast;
		chunkNode* pFirst;
		
	public:
		chunkQueue();

		void push(chunkNode* tempChunk);

		chunkNode* peek();
		
		bool isEmpty();
		
		chunkNode* pop();

		~chunkQueue();
};

class ipc
{
	protected:
	
		int m_fdTx;
		
		int m_fdRx;
	
	public:
		ipc();
		
	    virtual const int getReadFd();
	    
	    virtual const int getWriteFd();
	    
		virtual void preparePolling();
		
	    virtual void processRx();
		
		virtual void processTx();
		
		virtual bool isWritePending();
		
		~ipc();
};

struct ipcNode
{
	ipc*		pPipe;
	ipcNode*	pNext;
};

class ipcQueue
{
	protected:
		ipcNode*	pLast;
		ipcNode*	pFirst;
		
	public:
		ipcQueue();

		void push(ipcNode* tempChunk);

		ipcNode* peek();
		
		ipcNode* pop();

		~ipcQueue();
};

class ipcPipe : public ipc
{
    protected:
    
	    const char* m_pfnWrite;
	    const char* m_pfnRead;
	    
	    const char* m_pWrkDir;

		chunkQueue m_TxChunkQueue;
		chunkQueue m_RxChunkQueue;
        
        int m_nfdWriteMax;
        int m_nfdReadMax;
		
    public:
        
	    ipcPipe(const char* pfnWrite, const char* pfnRead);
	    
	    int setup();
	
	    virtual void preparePolling();
	    
	    int txData(const char* pSrcbuffer, int nLength);
		
	    int rxData(char* pDstbuffer, int nMaxLength);
	    
	    virtual void processRx();
		
		virtual void processTx();
		
		virtual bool isWritePending();
	    
	    ~ipcPipe();
};

class ipcProcess : public ipc
{
    protected:
	    
	    const char* m_pStrCmd;
	    const char* m_pWrkDir;
        int	m_fdPtyMaster;
        int m_pidChild;

		chunkQueue m_TxChunkQueue;
		chunkQueue m_RxChunkQueue;
        
        int m_nfdWriteMax;
        int m_nfdReadMax;
		
    public:
        
	    ipcProcess(const char* pStrCmd, const char* pWrkDir);
	    
	    int setup();
	    
	    virtual const int getReadFd();
	    
	    virtual const int getWriteFd();
	
	    virtual void preparePolling();
	    
	    int txData(const char* pSrcbuffer, int nLength);
		
	    int rxData(char* pDstbuffer, int nMaxLength);
	    
	    virtual void processRx();
		
		virtual void processTx();
		
		virtual bool isWritePending();
		
		virtual void exec(char* const args[]);
	    
	    ~ipcProcess();
};

class ipcHandler
{
	protected:

        int m_nfdReadMax;
        int m_nfdWriteMax;
        
        fd_set m_fdsetRead;
        fd_set m_fdsetWrite;
        
        struct timeval m_tv;
        
        ipcQueue	m_IpcQueue;
        
	public:
        
		ipcHandler();
		
		void add(ipc* p);
		
		void preparePolling();
		
		void transact();
        
		~ipcHandler();
};
