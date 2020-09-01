#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>

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

msgQueue::msgQueue()
{
	rear = NULL;
	front = NULL;
}

void msgQueue::push(msgNode* tempMsg)
{
	tempMsg->next = NULL;
	
	if(front == NULL)
	{
		front = tempMsg;
		rear = tempMsg;
	}
	else{
		rear->next = tempMsg;
		rear = tempMsg;
	}
}

msgNode* msgQueue::peek()
{
	return front;
}

bool msgQueue::isEmpty()
{
	bool bRet = false;
	
	if(NULL == front)
	{
		bRet = true;
	} 
	
	return bRet;
}

msgNode* msgQueue::pop()
{
	msgNode* tempMsg;
	
	if(front == NULL)
	{
		tempMsg = NULL;
	}
	
	if(front == rear)
	{
		tempMsg = front;
		front = rear = NULL;
	}
	else
	{
		tempMsg = front;
		front = front->next;
	}
	
	return tempMsg;
}

msgQueue::~msgQueue()
{
	while(front!=NULL)
	{
		msgNode *temp=front;
		front=front->next;
		delete temp;
	}
	
	rear=NULL;
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
	rear = NULL;
	front = NULL;
}

void ipcQueue::push(ipcNode* tempMsg)
{
	tempMsg->next = NULL;
	
	if(front == NULL)
	{
		front = tempMsg;
		rear = tempMsg;
	}
	else{
		rear->next = tempMsg;
		rear = tempMsg;
	}
}

ipcNode* ipcQueue::peek()
{
	return front;
}

ipcNode* ipcQueue::pop()
{
	ipcNode* tempMsg;
	
	if(front == NULL)
	{
		//underflow
		tempMsg = NULL;
	}
	if(front == rear)
	{
		tempMsg = front;
		front = rear = NULL;
	}
	else
	{
		tempMsg = front;
		front = front->next;
	}
	
	return tempMsg;
}

ipcQueue::~ipcQueue()
{
	while(front!=NULL)
	{
		ipcNode *temp=front;
		front=front->next;
		delete temp;
	}
	
	rear=NULL;
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
			if(MAX_MSG_LENGTH < nWriteLen)
			{
				// Over write 
				nWriteLen = MAX_MSG_LENGTH;
			}
			
			msgNode* pTmpMsg = new msgNode;
			
			pTmpMsg->length = nWriteLen;
			memcpy(pTmpMsg->buffer, pSrcbuffer, nWriteLen);
				
			m_TxMsgQueue.push(pTmpMsg);
			
			nLength = nLength - nWriteLen;
			pSrcbuffer = pSrcbuffer + nLength;
		}
	}
	
	return nWriteLen;
}

bool ipcPipe::isWritePending()
{
	bool bRet = !m_TxMsgQueue.isEmpty();
	
	return bRet;
}

int ipcPipe::rxData(char* pDstbuffer, int nMaxLength)
{
	int nReadLen = -1;
	msgNode* pTmpMsg = m_RxMsgQueue.peek();
	
	if(NULL != pTmpMsg)
	{
		if(0 < pTmpMsg->length)
		{
			nReadLen = pTmpMsg->length;
			if(nMaxLength < nReadLen)
			{
				// Under read
				nReadLen = nMaxLength;
			}
			
			memcpy(pDstbuffer, pTmpMsg->buffer, nReadLen);
			
			// Todo: fix under read		
			pTmpMsg->length = pTmpMsg->length - nReadLen;	
			if(0 < pTmpMsg->length)
			{
				memcpy(pTmpMsg->buffer, pTmpMsg->buffer + nReadLen, pTmpMsg->length);
			}
		}
		
		//free msg if everything is read
		if(0 == pTmpMsg->length)
		{
			// pTmpMsg is orwriteen with same value to avoid compiler warning
			// pop is necessary
			pTmpMsg = m_RxMsgQueue.pop();
			
			free(pTmpMsg);
			pTmpMsg = NULL;
		}
	}
	
	return nReadLen;
}

void ipcPipe::processRx()
{
	msgNode* pOutputMsg = new msgNode;
	
	pOutputMsg->length = read(m_fdRx, pOutputMsg->buffer, MAX_MSG_LENGTH);
	
	if(0 < pOutputMsg->length)
	{
		//push msg into output msgQueue
		m_RxMsgQueue.push(pOutputMsg);
		
		//printf("Output msg length: %d\n",pOutputMsg->length);
	}
}

void ipcPipe::processTx()
{
	msgNode* pTmpMsg = m_TxMsgQueue.pop();
	
	if(NULL != pTmpMsg)
	{
		if(0 < pTmpMsg->length)
		{
			write(m_fdTx, pTmpMsg->buffer, pTmpMsg->length);
			//TODO: Use fsync only for unbuffered transfers
			fsync(m_fdTx);
		}
		
		//free msg
		free(pTmpMsg);
		pTmpMsg = NULL;
	}
}

ipcPipe::~ipcPipe()
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

void ipcHandler::addNewIpc(ipc* pPipe)
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
		
		pPipeNode = pPipeNode->next;
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
					
				pPipeNode = pPipeNode->next; 
			}
		}
	}
}

ipcHandler::~ipcHandler()
{
	
}


