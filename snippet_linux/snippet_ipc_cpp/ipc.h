
#define MAX_MSG_LENGTH 128

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

//

#define m_TX_s_RX_Pipe "m_tx_s_rx_uart.fifo"

#define s_TX_m_RX_Pipe "s_tx_m_rx_uart.fifo"

//

int msleep(long msec);

struct msgNode
{
	int length;
	char buffer[MAX_MSG_LENGTH];
	msgNode* next;
};

// Credit Ref: https://www.techiedelight.com/queue-implementation-cpp/
class msgQueue
{
	protected:
		msgNode* rear;
		msgNode* front;
		
	public:
		msgQueue();

		void push(msgNode* tempMsg);

		msgNode* peek();
		
		bool isEmpty();
		
		msgNode* pop();

		~msgQueue();
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
	ipc*	   pPipe;
	ipcNode* next;
};

class ipcQueue
{
	protected:
		ipcNode* rear;
		ipcNode* front;
		
	public:
		ipcQueue();

		void push(ipcNode* tempMsg);

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

		msgQueue m_TxMsgQueue;
		msgQueue m_RxMsgQueue;
        
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

		msgQueue m_TxMsgQueue;
		msgQueue m_RxMsgQueue;
        
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
