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

#include "app.h" 
#include "ipc.h" 

void processMsgs(ipcPipe* pPipe)
{
	char tmpBuffer[MAX_CHUNK_LENGTH+1];
	int	nReadLen = -1;
	int	nWriteLen = -1;
	
	// Reading part
	nReadLen = pPipe->rxData(tmpBuffer, MAX_CHUNK_LENGTH);
	if(0 < nReadLen) 
	{
		// Do something with the read data
		tmpBuffer[nReadLen] = '\0';
		printf("Recd s: %s\n", tmpBuffer);
	}
	
	// Writing part
	// example write
	if(0 == strcmp(tmpBuffer, "hello !"))
	{
		const char* pStrSample = "bye !";
		int nlen =  -1;
		
		nlen = strlen(pStrSample);
		memcpy(tmpBuffer, pStrSample, nlen);
		pPipe->txData(tmpBuffer, nlen);
	}
}

int main()
{
	bool bDone = false;
	
	char tmpBuffer[MAX_CHUNK_LENGTH+1];
	
    ipcHandler* pPipeHandler = new ipcHandler();
    
    ipcPipe* pPipe = new ipcPipe(s_TX_m_RX_Pipe, m_TX_s_RX_Pipe);
    
    pPipe->setup();
    
    pPipeHandler->add(pPipe);
    
    msleep(2000);
    
    {
		const char* pStrSample = "hi !";
		int nlen =  -1;
		
		nlen = strlen(pStrSample);
		memcpy(tmpBuffer, pStrSample, nlen);
		pPipe->txData(tmpBuffer, nlen);
	}
    
    while(false == bDone)
    {
		pPipeHandler->transact();
		
		processMsgs(pPipe);
	}

    delete pPipe;
    pPipe = NULL;

    delete pPipeHandler;
    pPipeHandler = NULL;
    
    return 0;
}
