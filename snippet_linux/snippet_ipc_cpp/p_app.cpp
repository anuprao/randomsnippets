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

void processMsgsOnPty(ipcProcess* pProcess)
{
	char tmpBuffer[MAX_MSG_LENGTH+1];
	int	nReadLen = -1;
	int	nWriteLen = -1;
	
	// Reading part
	nReadLen = pProcess->rxData(tmpBuffer, MAX_MSG_LENGTH);
	//printf("nReadLen : %d\n", nReadLen);
	if(0 < nReadLen) 
	{
		// Do something with the read data
		tmpBuffer[nReadLen] = '\0';
		printf("Recd m: %s\n", tmpBuffer);
	}
	
	// Writing part
	// example write
	/*
	if(0 == strcmp(tmpBuffer, "hi !"))
	{
		const char* pStrSample = "hello !";
		int nlen =  -1;
		
		nlen = strlen(pStrSample);
		memcpy(tmpBuffer, pStrSample, nlen);
		pProcess->txData(tmpBuffer, nlen);
	}
	* */
}

int main()
{
	char* const args[] = {"python3", "test.py", NULL}; 
	
	int pidChild;
	
	bool bDone = false;
	
    ipcHandler* pPipeHandler = new ipcHandler();
    
    ipcProcess* pProcess = new ipcProcess("", "");
    
    pidChild = pProcess->setup();
    
    pPipeHandler->add(pProcess);
    
    if(0 == pidChild)
    {
		//printf("In child\r\n");
		
		pProcess->exec(args);
	}
	else
	{
		//printf("In parent with pidChild : %d\r\n", pidChild);
		
		while(false == bDone)
		{
			pPipeHandler->transact();
			
			processMsgsOnPty(pProcess);
		}
	}

    delete pProcess;
    pProcess = NULL;

    delete pPipeHandler;
    pPipeHandler = NULL;
    
    return 0;
}
