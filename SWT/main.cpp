#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "Funktionen.h"
#include <stdint.h>
#include <fstream>
#include <time.h>
#include <sstream>
#include <iostream>
#include <direct.h>
#define COM_BUFFER_SIZE 256       // Read- und Write-Buffer-Size
#define BD_RATE         CBR_4800 // 9600 Baud

// Hauptprogramm

int main (int argc, char **argv)

{
    Globaldata globaldata;
    GUI gui;
    CONTROL control;
    
	gui.requestComport(globaldata.comport);
    gui.requestIP(globaldata.IPEndstellen);
    control.getFileHandle(globaldata);
    
    if (control.isFileHandle(globaldata)){
    	gui.FileHandleOK(globaldata);
	}else{
		return (1); // exit
	}
    
	control.setDataForhCom(globaldata);
    SetCommState (globaldata.hCom, &globaldata.dcb);    // COM-Einstellungen speichern
    GetCommTimeouts (globaldata.hCom, &globaldata.ct);

    // Warte-Zeit [ms] vom Beginn eines Bytes bis zum Beginn des nächsten Bytes
    control.setTimer(globaldata);

    // Zwischenspeicher des serial-Drivers einstellen (für read und write):
    SetupComm (globaldata.hCom, COM_BUFFER_SIZE, COM_BUFFER_SIZE);
    SetCommMask (globaldata.hCom, globaldata.dwSetMask); // globaldata.empfangssignale definieren

    //char Test[256];
    char Slogpfad[256]; //hier wird der Programmpfad gespeichert
    getcwd(Slogpfad, 256); //der Programmpfad ist jetzt in 'temp' gespeichert
    strcat(Slogpfad, "\\LWLog.csv"); 
    std::ofstream outFile0(Slogpfad);           
    outFile0 << "Datum Zeit" << ";" << "Leitwert" << ";" << "Temperatur" <<std::endl;
    outFile0.close();
    char Zielpfad[50];

    strcpy(Zielpfad,"//tbtvsrv01/DATASHARE/SPS_Daten/");
    strcat(Zielpfad,globaldata.IPEndstellen);
    strcat(Zielpfad,"/LW.txt");

    globaldata.send[0] = 0xFE;
    globaldata.send[1] = 0x00;
    globaldata.send[2] = 0x3D;

    printf("Sende %2X%2X%2X\n\r",globaldata.send[0],globaldata.send[1],globaldata.send[2]);  

    // Senden des Sendestrings Initial
    WriteFile (globaldata.hCom, &globaldata.send, 3, &globaldata.iBytesWritten, NULL);

        do  // in Endlos-Schleife auf Empfangssignale warten:

        {
            WaitCommEvent (globaldata.hCom, &globaldata.dwEvtMask, &globaldata.o); // Event mit globaldata.empfangssignalen verknüpfen

               if (WAIT_OBJECT_0 == WaitForSingleObject (globaldata.o.hEvent, INFINITE)) // warten bis Event

               {            
                    if (globaldata.dwEvtMask & EV_RXCHAR) // Zeichen an RxD empfangen:

                    {
                        globaldata.bRet = ReadFile (globaldata.hCom, globaldata.empfang, 100, &globaldata.dwRead,NULL);
                                                                                                            
                        if (!globaldata.bRet)
                        { // Fehlerausgabe:

                            LPVOID lpMsgBuf;
                            FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                            FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(),
                            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                            (LPTSTR) &lpMsgBuf, 0, NULL);                     
                            MessageBox (NULL, (LPCTSTR)lpMsgBuf, "Error: ReadFile", MB_OK | MB_ICONINFORMATION);
                            LocalFree (lpMsgBuf);
                        }
                        else
                        { // Daten erhalten ...

                        switch (globaldata.send[0]){

                            case 0xFE:

                              if(globaldata.empfang[0] == 254 && (globaldata.empfang[1] == 5 || globaldata.empfang[1] == 3 )&& (globaldata.empfang[2] == 38 || globaldata.empfang[2] == 52) && (globaldata.empfang[3] == 129 || globaldata.empfang[3] == 183 || globaldata.empfang[3] == 121 || globaldata.empfang[3] == 120 ) && globaldata.send[0] == 0xFE && globaldata.send[2] == 0x3D)

                              {

                            	// globaldata.empfangene Daten zu leitwert Decodieren

                                globaldata.LW_now = (256*(255-globaldata.empfang[6]))+globaldata.empfang[7];

                                printf("Ausgabe globaldata.LW_now  %d \n\r",globaldata.LW_now);   

                                if (globaldata.LW_now < globaldata.LW +30  )

                                {

                                    globaldata.LW = globaldata.LW_now;

                                    globaldata.LWF = globaldata.LW_now*1.0;

                                }

                                printf ("Log Aktiv Aktueller Leitwert: %.1f \n\r", globaldata.LWF);

                            }

                             // Sendestring ändern (Temp Anfrage)

                            globaldata.send[0] = 0xFD;
                            globaldata.send[1] = 0x00;
                            globaldata.send[2] = 0x02;                            

                            break;

                            case 0xFD:

                                if(globaldata.empfang[0] == 253 && (globaldata.empfang[1] == 5 || globaldata.empfang[1] == 3) && (globaldata.empfang[2] == 25 || globaldata.empfang[2] == 11) && (globaldata.empfang[3] == 121 || globaldata.empfang[3] == 183|| globaldata.empfang[3] == 182 || globaldata.empfang[3] == 120) && globaldata.send[0] == 0xFD && globaldata.send[2] == 0x02)

                                    {

                                        // globaldata.empfangene Daten zu Temp Decodieren
                                        globaldata.Temp = (256*(255-globaldata.empfang[6]))+globaldata.empfang[7];
                                        globaldata.TempF = globaldata.Temp*0.1;
                                        // Sendestring ändern (LW Anfrage)
                                        // Consolenzeile "löschen"k
                                        for( int i=0; i<60; ++i )

                                        {
                                            putchar( '\b' );
                                        }

                                        // Consolenausgabe

                                        printf ("Log Aktiv, Aktuelle Temperatur:  %.1f C \n\r", globaldata.TempF);

                                        // Wert in Serverfile schreiben                                                                                                                                                                                                                                                                            
                                        std::ofstream outFile(Zielpfad);
                                        outFile << globaldata.LW << 'L' << globaldata.Temp << 'T' <<std::endl;
                                        outFile.close();

                                        // simple Logfile erstellen .............................................

                                    	if(globaldata.simple_log != 1){
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 
                                            time_t Zeitstempel;
                                            tm *nun;
                                            Zeitstempel = time(0);
                                            nun = localtime(&Zeitstempel);
                                            char LWSTR[5];
                                            float_to_string(globaldata.LWF,LWSTR);
                                            char TempSTR[5];
                                            float_to_string(globaldata.TempF,TempSTR);
                                                                                                                                                                   
                                            char *c;
                                            while((c = strchr(LWSTR, '.'))!=NULL)*c = ',';
                                            while((c = strchr(TempSTR, '.'))!=NULL)*c = ',';          
                                            if(globaldata.empfang[1] == 5){

                                            	printf("\n\rGreisinger GMH3431 erkannt LW: %.2f Temp:%.2f\n\r",globaldata.LWF *0.1,globaldata.TempF);

                                            }                                                                                                                           

                                            if(globaldata.empfang[1] == 3){

	                                             printf("\n\r**************************************************************************\n\r");
	
	                                             printf("Greisinger GMH3410 erkannt es wird nur das Geraet GMH 3431 unterstuetzt!\n\r");
	
	                                             printf("**************************************************************************\n\r");
                                                 //exit(0);
                                                                            
                                            }                                                                                                                                                                                                                                                                                                                        
                                            std::ofstream outFile2(Slogpfad, std::ios::app );           
                                            outFile2 << nun->tm_mday << "." << nun->tm_mon+1 << "." << nun->tm_year+1900 << " " << nun->tm_hour << ":" << nun->tm_min << ":" << nun->tm_sec << ";" << LWSTR << ";" << TempSTR << std::endl;
                                            outFile2.close();                                                                                                                                                                           
                                            }
                                              //..............................................................................

                                             // 900 ms warten           

                                              }
                                            globaldata.send[0] = 0xFE;
                                  			globaldata.send[1] = 0x00;
                                  			globaldata.send[2] = 0x3D;
                                            break;        
                                            default:
                                            globaldata.send[0] = 0xFD;
                                      		globaldata.send[1] = 0x00;
                                      		globaldata.send[2] = 0x02; 
                                            break;
             }                                                                                                                
                                    // Sendeanfrage erneut senden

                                    WriteFile (globaldata.hCom, &globaldata.send, 3, &globaldata.iBytesWritten, NULL); // Senden der Bytes

                                            Sleep(900);       
                               }
                       }

                       if (globaldata.dwEvtMask & EV_ERR)
                       {
                               MessageBox (NULL, "Error empfangen", "Error: ReadFile", MB_OK);
                               break; // Schleifen-Abbruch
                       }
               }

        }

        while (1);
        CloseHandle (globaldata.hCom);     // COM schließen
        CloseHandle (globaldata.o.hEvent); // Event-Handle zurückgeben
        return (0);
}


