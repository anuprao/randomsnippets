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

void processMsgsOnPipe(ipcPipe* pPipe)
{
	char tmpBuffer[MAX_MSG_LENGTH+1];
	int	nReadLen = -1;
	int	nWriteLen = -1;
	
	// Reading part
	nReadLen = pPipe->rxData(tmpBuffer, MAX_MSG_LENGTH);
	if(0 < nReadLen) 
	{
		// Do something with the read data
		tmpBuffer[nReadLen] = '\0';
		printf("Recd m: %s\n", tmpBuffer);
	}
	
	// Writing part
	// example write
	if(0 == strcmp(tmpBuffer, "hi !"))
	{
		const char* pStrSample = "hello !";
		int nlen =  -1;
		
		nlen = strlen(pStrSample);
		memcpy(tmpBuffer, pStrSample, nlen);
		pPipe->txData(tmpBuffer, nlen);
	}
}

int main()
{
	bool bDone = false;
	
    ipcHandler* pRedirHandler = new ipcHandler();
    
    ipcPipe* pPipe = new ipcPipe(m_TX_s_RX_Pipe, s_TX_m_RX_Pipe);
    
    pPipe->setup();
    
    pRedirHandler->addNewRedir(pPipe);
    
    while(false == bDone)
    {
		pRedirHandler->transact();
		
		processMsgsOnPipe(pPipe);
	}

    delete pPipe;
    pPipe = NULL;

    delete pRedirHandler;
    pRedirHandler = NULL;
    
    return 0;
}
