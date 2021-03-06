#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <sys/ioctl.h>

#if 1
	#include <pty.h>
#else
	#include <util.h>
#endif

#include "ipc.h" 

int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do 
    {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

chunkQueue::chunkQueue()
{
	pLast = NULL;
	pFirst = NULL;
}

void chunkQueue::push(chunkNode* tempChunk)
{
	tempChunk->pNext = NULL;
	
	if(NULL == pFirst)
	{
		pFirst = tempChunk;
		pLast = tempChunk;
	}
	else{
		pLast->pNext = tempChunk;
		pLast = tempChunk;
	}
}

chunkNode* chunkQueue::peek()
{
	return pFirst;
}

bool chunkQueue::isEmpty()
{
	bool bRet = false;
	
	if(NULL == pFirst)
	{
		bRet = true;
	} 
	
	return bRet;
}

chunkNode* chunkQueue::pop()
{
	chunkNode* tempChunk;
	
	if(NULL == pFirst)
	{
		tempChunk = NULL;
	}
	
	if(pFirst == pLast)
	{
		tempChunk = pFirst;
		pFirst = pLast = NULL;
	}
	else
	{
		tempChunk = pFirst;
		pFirst = pFirst->pNext;
	}
	
	return tempChunk;
}

chunkQueue::~chunkQueue()
{
	while(NULL != pFirst)
	{
		chunkNode *temp=pFirst;
		pFirst=pFirst->pNext;
		
		delete temp;
	}
	
	pLast=NULL;
}

ipc::ipc()
{
	m_fdRx = -1;
	m_fdTx = -1;
}

const int ipc::getReadFd()
{
	return m_fdRx;
}

const int ipc::getWriteFd()
{
	return m_fdTx;
}

void ipc::preparePolling()
{
		  
}

void ipc::processRx()
{

}

void ipc::processTx()
{

}

bool ipc::isWritePending()
{
	bool bRet = false;
	
	return bRet;
}

ipc::~ipc()
{
	
}



ipcQueue::ipcQueue()
{
	pLast = NULL;
	pFirst = NULL;
}

void ipcQueue::push(ipcNode* tempChunk)
{
	tempChunk->pNext = NULL;
	
	if(NULL == pFirst)
	{
		pFirst = tempChunk;
		pLast = tempChunk;
	}
	else{
		pLast->pNext = tempChunk;
		pLast = tempChunk;
	}
}

ipcNode* ipcQueue::peek()
{
	return pFirst;
}

ipcNode* ipcQueue::pop()
{
	ipcNode* tempChunk;
	
	if(NULL == pFirst)
	{
		//underflow
		tempChunk = NULL;
	}
	if(pFirst == pLast)
	{
		tempChunk = pFirst;
		pFirst = pLast = NULL;
	}
	else
	{
		tempChunk = pFirst;
		pFirst = pFirst->pNext;
	}
	
	return tempChunk;
}

ipcQueue::~ipcQueue()
{
	while(NULL != pFirst)
	{
		ipcNode *temp=pFirst;
		pFirst=pFirst->pNext;
		delete temp;
	}
	
	pLast=NULL;
}

ipcPipe::ipcPipe(const char* pfnWrite, const char* pfnRead)
{	
	m_pfnWrite = pfnWrite;
	m_pfnRead = pfnRead;
	m_pWrkDir = NULL;
	
	m_nfdWriteMax = -1;
	m_nfdReadMax = -1;
}

int ipcPipe::setup()
{
	m_nfdWriteMax = 0;
	m_nfdReadMax = 0;
	
	m_fdTx = open(m_pfnWrite, O_RDWR);
	if(-1 == m_fdTx)
	{
		printf("Error in opening ipcPipe %s\n", m_pfnWrite);
		return (1);
	}
	
	m_fdRx = open(m_pfnRead, O_RDWR);
	if(-1 == m_fdRx)
	{
		printf("Error in opening ipcPipe %s\n", m_pfnRead);
		return (1);
	}
	
	return 0;
}
	    
void ipcPipe::preparePolling()
{
	//printf("m_fdTx %d\n", m_fdTx);
	//printf("m_fdRx %d\n", m_fdRx);
	
	m_nfdWriteMax = MAX(m_nfdWriteMax, m_fdTx);
	m_nfdReadMax = MAX(m_nfdReadMax, m_fdRx);
	
	//printf("m_nfdWriteMax %d\n", m_nfdWriteMax);
	//printf("m_nfdReadMax %d\n", m_nfdReadMax);  		  
}

int ipcPipe::txData(const char* pSrcbuffer, int nLength)
{
	int nWriteLen = -1;
	if(NULL != pSrcbuffer)
	{
		while(0 < nLength)
		{
			nWriteLen = nLength;
			if(MAX_CHUNK_LENGTH < nWriteLen)
			{
				// Over write 
				nWriteLen = MAX_CHUNK_LENGTH;
			}
			
			chunkNode* pTmpChunk = new chunkNode;
			
			pTmpChunk->length = nWriteLen;
			memcpy(pTmpChunk->buffer, pSrcbuffer, nWriteLen);
				
			m_TxChunkQueue.push(pTmpChunk);
			
			nLength = nLength - nWriteLen;
			pSrcbuffer = pSrcbuffer + nWriteLen;
		}
	}
	
	return nWriteLen;
}

bool ipcPipe::isWritePending()
{
	bool bRet = !m_TxChunkQueue.isEmpty();
	
	return bRet;
}

int ipcPipe::rxData(char* pDstbuffer, int nMaxLength)
{
	int nReadLen = -1;
	chunkNode* pTmpChunk = m_RxChunkQueue.peek();
	
	if(NULL != pTmpChunk)
	{
		if(0 < pTmpChunk->length)
		{
			nReadLen = pTmpChunk->length;
			if(nMaxLength < nReadLen)
			{
				// Under read
				nReadLen = nMaxLength;
			}
			
			memcpy(pDstbuffer, pTmpChunk->buffer, nReadLen);
			
			// Todo: fix under read		
			pTmpChunk->length = pTmpChunk->length - nReadLen;	
			if(0 < pTmpChunk->length)
			{
				memcpy(pTmpChunk->buffer, pTmpChunk->buffer + nReadLen, pTmpChunk->length);
			}
		}
		
		//free chunk if everything is read
		if(0 == pTmpChunk->length)
		{
			// pTmpChunk is orwriteen with same value to avoid compiler warning
			// pop is necessary
			pTmpChunk = m_RxChunkQueue.pop();
			
			free(pTmpChunk);
			pTmpChunk = NULL;
		}
	}
	
	return nReadLen;
}

void ipcPipe::processRx()
{
	chunkNode* pOutputChunk = new chunkNode;
	
	pOutputChunk->length = read(m_fdRx, pOutputChunk->buffer, MAX_CHUNK_LENGTH);
	
	if(0 < pOutputChunk->length)
	{
		//push chunk into output chunkQueue
		m_RxChunkQueue.push(pOutputChunk);
		
		//printf("Output chunk length: %d\n",pOutputChunk->length);
	}
}

void ipcPipe::processTx()
{
	chunkNode* pTmpChunk = m_TxChunkQueue.pop();
	
	if(NULL != pTmpChunk)
	{
		if(0 < pTmpChunk->length)
		{
			write(m_fdTx, pTmpChunk->buffer, pTmpChunk->length);
			//TODO: Use fsync only for unbuffered transfers
			//fsync(m_fdTx);
		}
		
		//free chunk
		free(pTmpChunk);
		pTmpChunk = NULL;
	}
}

ipcPipe::~ipcPipe()
{
}
 
ipcProcess::ipcProcess(const char* pStrCmd, const char* pWrkDir)
{		    
	m_pStrCmd = pStrCmd;
	m_pWrkDir = pWrkDir;
	m_fdPtyMaster = -1;
	m_pidChild = -1;
	
	m_nfdWriteMax = -1;
	m_nfdReadMax = -1;
}

int ipcProcess::setup()
{
	m_nfdWriteMax = 0;
	m_nfdReadMax = 0;
	
	m_pidChild = forkpty(&m_fdPtyMaster, NULL, NULL, NULL);
	
	return m_pidChild;
}

const int ipcProcess::getReadFd()
{
	return m_fdPtyMaster;
}

const int ipcProcess::getWriteFd()
{
	return m_fdPtyMaster;
}
	    
void ipcProcess::preparePolling()
{
	struct termios tios;
	
	tcgetattr(m_fdPtyMaster, &tios);
	tios.c_lflag &= ~(ECHO | ECHONL);
	tcsetattr(m_fdPtyMaster, TCSAFLUSH, &tios);
	
	m_nfdWriteMax = MAX(m_nfdWriteMax, m_fdPtyMaster);
	m_nfdReadMax = MAX(m_nfdReadMax, m_fdPtyMaster);
	
	//printf("m_nfdWriteMax %d\n", m_nfdWriteMax);
	//printf("m_nfdReadMax %d\n", m_nfdReadMax);  		  
}

int ipcProcess::txData(const char* pSrcbuffer, int nLength)
{
	int nWriteLen = -1;
	if(NULL != pSrcbuffer)
	{
		while(0 < nLength)
		{
			nWriteLen = nLength;
			if(MAX_CHUNK_LENGTH < nWriteLen)
			{
				// Over write 
				nWriteLen = MAX_CHUNK_LENGTH;
			}
			
			chunkNode* pTmpChunk = new chunkNode;
			
			pTmpChunk->length = nWriteLen;
			memcpy(pTmpChunk->buffer, pSrcbuffer, nWriteLen);
				
			m_TxChunkQueue.push(pTmpChunk);
			
			nLength = nLength - nWriteLen;
			pSrcbuffer = pSrcbuffer + nWriteLen;
		}
	}
	
	return nWriteLen;
}

bool ipcProcess::isWritePending()
{
	bool bRet = !m_TxChunkQueue.isEmpty();
	
	return bRet;
}

int ipcProcess::rxData(char* pDstbuffer, int nMaxLength)
{
	int nReadLen = -1;
	chunkNode* pTmpChunk = m_RxChunkQueue.peek();
	
	if(NULL != pTmpChunk)
	{
		if(0 < pTmpChunk->length)
		{
			nReadLen = pTmpChunk->length;
			if(nMaxLength < nReadLen)
			{
				// Under read
				nReadLen = nMaxLength;
			}
			
			memcpy(pDstbuffer, pTmpChunk->buffer, nReadLen);
			
			// Todo: fix under read		
			pTmpChunk->length = pTmpChunk->length - nReadLen;	
			if(0 < pTmpChunk->length)
			{
				memcpy(pTmpChunk->buffer, pTmpChunk->buffer + nReadLen, pTmpChunk->length);
			}
		}
		
		//free chunk if everything is read
		if(0 == pTmpChunk->length)
		{
			// pTmpChunk is orwritten with same value to avoid compiler warning
			// pop is necessary
			pTmpChunk = m_RxChunkQueue.pop();
			
			free(pTmpChunk);
			pTmpChunk = NULL;
		}
	}
	
	return nReadLen;
}

void ipcProcess::processRx()
{
	chunkNode* pOutputChunk = new chunkNode;
	
	pOutputChunk->length = read(m_fdPtyMaster, pOutputChunk->buffer, MAX_CHUNK_LENGTH);
	
	if(0 < pOutputChunk->length)
	{
		//push chunk into output chunkQueue
		m_RxChunkQueue.push(pOutputChunk);
		
		//printf("Output chunk length: %d\n",pOutputChunk->length);
	}
}

void ipcProcess::processTx()
{
	chunkNode* pTmpChunk = m_TxChunkQueue.pop();
	
	if(NULL != pTmpChunk)
	{
		if(0 < pTmpChunk->length)
		{
			write(m_fdPtyMaster, pTmpChunk->buffer, pTmpChunk->length);
			//TODO: Use fsync only for unbuffered transfers
			//fsync(m_fdPtyMaster);
		}
		
		//free chunk
		free(pTmpChunk);
		pTmpChunk = NULL;
	}
}

void ipcProcess::exec(char* const args[])
{
	execvp(args[0], args); 
	
	printf("Alert ! Check child process execution !!\n");
	exit(1);
}
	    
ipcProcess::~ipcProcess()
{
} 
        
ipcHandler::ipcHandler()
{	
	m_nfdReadMax = -1;
	m_nfdWriteMax = -1;
	
	FD_ZERO(&m_fdsetRead);
	FD_ZERO(&m_fdsetWrite);
	
	m_tv.tv_sec = 0;
	m_tv.tv_usec = 0;
}

void ipcHandler::add(ipc* pPipe)
{
	if(NULL != pPipe)
	{
		ipcNode* pPipeNode = new ipcNode;
		pPipeNode->pPipe = pPipe;
		m_IpcQueue.push(pPipeNode);
	}
}

void ipcHandler::preparePolling()
{
	m_nfdReadMax = -1;
	m_nfdWriteMax = -1;
	
	ipcNode* pPipeNode = NULL;
	
	FD_ZERO(&m_fdsetRead);
	FD_ZERO(&m_fdsetWrite);
		
	pPipeNode = m_IpcQueue.peek();
	while(NULL != pPipeNode)
	{	
		ipc* pPipe = pPipeNode->pPipe;
		if(NULL != pPipe)
		{
			const int fdRead = pPipe->getReadFd();
			const int fdWrite = pPipe->getWriteFd();
				
			pPipe->preparePolling();
			
			m_nfdReadMax = MAX(m_nfdReadMax, fdRead);
			m_nfdWriteMax = MAX(m_nfdWriteMax, fdWrite);
			
			FD_SET(fdRead, &m_fdsetRead);
			
			if(true == pPipe->isWritePending())
			{
				FD_SET(fdWrite, &m_fdsetWrite);  
			}
		}
		
		pPipeNode = pPipeNode->pNext;
	}	  
}

void ipcHandler::transact()
{
	int ready_fd;
	
	int n_fd;
	int i;
	
	//printf("transact with child\n");
	preparePolling();
	 
	n_fd = MAX(m_nfdReadMax, m_nfdWriteMax) + 1;	
	
	{	
		//printf("Looping");
		m_tv.tv_sec = 0;
		m_tv.tv_usec = 2000;

		//printf("calling select with n_fd %d\n", n_fd);
		ready_fd = select(n_fd, &m_fdsetRead, &m_fdsetWrite, NULL, NULL);//&m_tv);
		
		//printf("readyfd: %d\n",ready_fd);
		if(ready_fd > 0)
		{
			ipcNode* pPipeNode = NULL;
			
			pPipeNode = m_IpcQueue.peek();
			while(NULL != pPipeNode)
			{	
				ipc* pPipe = pPipeNode->pPipe;
				if(NULL != pPipe)
				{
					const int fdRead = pPipe->getReadFd();
					const int fdWrite = pPipe->getWriteFd();
					
					if(FD_ISSET(fdRead, &m_fdsetRead))
					{	
						pPipe->processRx();
						
						FD_CLR(fdRead, &m_fdsetRead);
					}
					
					if(FD_ISSET(fdWrite, &m_fdsetWrite))
					{	
						pPipe->processTx();
						
						FD_CLR(fdWrite, &m_fdsetWrite);
					}
					
				}
					
				pPipeNode = pPipeNode->pNext; 
			}
		}
	}
}

ipcHandler::~ipcHandler()
{
	
}


