#include <windows.h>
#include <stdio.h>
#include <string>
#include <stdint.h>
#include <fstream>
#include <time.h>
#include <sstream>
#include <iostream>
#include <direct.h>
#define COM_BUFFER_SIZE 256       // Read- und Write-Buffer-Size
#define BD_RATE         CBR_4800 // 9600 Baud

class Globaldata {
	public:	int test;
	public: unsigned char 	send[3];
	public: uint8_t  		empfang[100];
	public: TCHAR        	szCOM[5];
	public: char       		comport[5];
	public: char  			IPEndstellen[5];
	public: int    			simple_log;
	public: int 			LW;
	public: int 			Temp;
	public: int 			step;
	public: float 			LWF;
	public: int 			LW_now;
    public: float 			TempF;
    public: DCB             dcb;   
    public: DWORD           iBytesWritten;
    public: BOOL            bRet;
    public: DWORD           dwRead;
    public: DWORD           dwWrite;
    public: DWORD           dwSetMask;
    public: DWORD           dwEvtMask;
    public: OVERLAPPED      o;
    public: COMMTIMEOUTS    ct;
	public: HANDLE 			hCom;
	    
	public:  Globaldata(){
		test = 99;	
		simple_log = 0;
		LW = 1000;
		step = 0;
		LWF = 1000.0;
		bRet      = true;
		dwRead    = 0;
		dwWrite    = 0;
		dwSetMask = EV_RXCHAR | EV_ERR;
	};	
};

