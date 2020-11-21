

 

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

                                              

        DCB           dcb;   

        DWORD         iBytesWritten;

        BOOL          bRet      = true;

        DWORD         dwRead    = 0;

        DWORD         dwWrite    = 0;

        DWORD         dwSetMask = EV_RXCHAR | EV_ERR;

        DWORD         dwEvtMask;

        OVERLAPPED    o;

        COMMTIMEOUTS  ct;

 

        unsigned char send[3];

        uint8_t  empfang[100];

        TCHAR        szCOM[5];

       char       comport[5];

       char  IPEndstellen[5];

       int    simple_log = 0;

       int LW = 1000;

       int Temp;

       int step = 0;

       float LWF = 1000.0;

       int LW_now;

       float TempF;

      

       printf ("Greisinger GMH 3400 einschalten und Datenkabel Verbinden");

       printf ("\r\nPort angeben (BsP.:COM1): ");

       scanf ("%5s",comport);

       printf ("\r\nGeraete IP Enstellen eingeben (BsP.: 65): ");

       scanf ("%5s",IPEndstellen);

       printf ("\r\n"); 

       strcpy(szCOM,comport);

 

 

        memset (&o, 0, sizeof (OVERLAPPED)); // Struktur mit 0en füllen

        o.hEvent = CreateEvent (NULL, FALSE, FALSE, NULL); // einen Event setzten

 

        HANDLE hCom = CreateFile (szCOM, GENERIC_WRITE | GENERIC_READ, 0, NULL,

                                       OPEN_EXISTING, 0, NULL);      

                                            

 

        if (hCom == INVALID_HANDLE_VALUE)

        { // Fehlerausgabe:

               LPVOID lpMsgBuf;

               FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(),

                              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);

               MessageBox (NULL, (LPCTSTR)lpMsgBuf, "Error: Conection", MB_OK | MB_ICONINFORMATION);

               LocalFree (lpMsgBuf);

               return (1); // und tschüß ...

        }else{

               printf("Connect = ok, Handel: %d\n\r",hCom);

        }

       

        

        dcb.DCBlength = sizeof(DCB);  // Laenge des Blockes MUSS gesetzt sein!

        GetCommState (hCom, &dcb);    // COM-Einstellungen holen und aendern

        dcb.BaudRate  = BD_RATE;      // Baudrate

        dcb.ByteSize  = 8;            // Datenbits

        dcb.Parity    = NOPARITY;     // Parität

        dcb.StopBits  = ONESTOPBIT;   // Stopbits

       dcb.fInX = false;

       dcb.fOutX = false;

       dcb.fOutxCtsFlow = false;

       dcb.fOutxDsrFlow = false;

       dcb.fDsrSensitivity = false;

       dcb.fAbortOnError = false;

       dcb.fBinary = true;

       dcb.fDtrControl = DTR_CONTROL_ENABLE;

       dcb.fRtsControl = RTS_CONTROL_DISABLE;

        SetCommState (hCom, &dcb);    // COM-Einstellungen speichern

        GetCommTimeouts (hCom, &ct);

       

        // Warte-Zeit [ms] vom Beginn eines Bytes bis zum Beginn des nächsten Bytes

        ct.ReadIntervalTimeout         = 1000 / BD_RATE * (dcb.ByteSize + (dcb.Parity == NOPARITY ? 0 : 1) + (dcb.StopBits == ONESTOPBIT ? 1 : 2)) * 2;

        ct.ReadTotalTimeoutMultiplier  = 0;  // [ms] wird mit Read-Buffer-Size multipliziert

        ct.ReadTotalTimeoutConstant    = 50; // wird an ReadTotalTimeoutMultiplier angehängt

        ct.WriteTotalTimeoutMultiplier = 0;

        ct.WriteTotalTimeoutConstant   = 0;

        SetCommTimeouts (hCom, &ct);

 

        // Zwischenspeicher des serial-Drivers einstellen (für read und write):

        SetupComm (hCom, COM_BUFFER_SIZE, COM_BUFFER_SIZE);

        SetCommMask (hCom, dwSetMask); // Empfangssignale definieren

 

       //char Test[256];

       char Slogpfad[256]; //hier wird der Programmpfad gespeichert

       getcwd(Slogpfad, 256); //der Programmpfad ist jetzt in 'temp' gespeichert

       //strcpy(Test,Slogpfad);

       strcat(Slogpfad, "\\LWLog.csv"); //in 'pfad' ist jetzt der absolute Pfad zu 'datei.txt' gespeichert.

       //strcat(Test, "\\LWLog_2.csv"); //in 'pfad' ist jetzt der absolute Pfad zu 'datei.txt' gespeichert.

       std::ofstream outFile0(Slogpfad);           

       outFile0 << "Datum Zeit" << ";" << "Leitwert" << ";" << "Temperatur" <<std::endl;

       outFile0.close();



       //FILE *fp;

       //fp = fopen( Test, "w+");



       char Zielpfad[50];

       //strcpy(Zielpfad,"//192.168.70.73/SPS_Daten/");

       strcpy(Zielpfad,"//tbtvsrv01/DATASHARE/SPS_Daten/");

       strcat(Zielpfad,IPEndstellen);

       strcat(Zielpfad,"/LW.txt");

       //printf ("\r\nPfad: %s",&Zielpfad);

  

  

  

        //Sendestring beschreiben Initial

        send[0] = 0xFE;

        send[1] = 0x00;

        send[2] = 0x3D;

        printf("Sende %2X%2X%2X\n\r",send[0],send[1],send[2]);  

                               // Senden des Sendestrings Initial

                               WriteFile (hCom, &send, 3, &iBytesWritten, NULL);

 

        do  // in Endlos-Schleife auf Empfangssignale warten:

        {

       

               WaitCommEvent (hCom, &dwEvtMask, &o); // Event mit Empfangssignalen verknüpfen

 

               if (WAIT_OBJECT_0 == WaitForSingleObject (o.hEvent, INFINITE)) // warten bis Event

               {            

                       if (dwEvtMask & EV_RXCHAR) // Zeichen an RxD empfangen:

                       {

                               bRet = ReadFile (hCom, empfang, 100, &dwRead,NULL);

                                                                                                                             

                               if (!bRet)

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

                              

                              //            printf("Daten erhalten ");  

                              //            printf("\r\nLW: ;%.1d; %.1d; %.1d; %.1d; %.1d; %.1d; %.1d; %.1d; %.1d; %.1d; %.1d", empfang[0], empfang[1], empfang[2], empfang[3], empfang[4], empfang[5], empfang[6], empfang[7], empfang[8], empfang[9], empfang[10]);            

                                             

                              switch (send[0]){

                            

                              case 0xFE:

                                                           

                              //printf("\r\nLW: ;0=%.1d; 1=%.1d; 2=%.1d; 3=%.1d; 4=%.1d; 5=%.1d; \n\r 6=%.1d; 7=%.1d; 8=%.1d; 9=%.1d; 10=%.1d; 11=%.1d;%X; %X;", empfang[0], empfang[1], empfang[2], empfang[3], empfang[4], empfang[5], empfang[6], empfang[7],empfang[8],empfang[9],empfang[10],empfang[11],send[0], send[2]);                                           

                              if(empfang[0] == 254 && (empfang[1] == 5 || empfang[1] == 3 )&& (empfang[2] == 38 || empfang[2] == 52) && (empfang[3] == 129 || empfang[3] == 183 || empfang[3] == 121 || empfang[3] == 120 ) && send[0] == 0xFE && send[2] == 0x3D)

                              {

                                            

                                             //printf("Ausgabe Step 0 %d  %d \n\r",empfang[6], empfang[7]);

                                             //fprintf(fp,"\r\nLog Aktiv, Aktueller Leitwert:; ;%.1f ;;bei;;; ;%.1f ;gradC", LWF, TempF);

                                             //fprintf(fp,"\r\nLW: ;%.1d; %.1d; %.1d; %.1d; %.1d; %.1d; %.1d; %.1d; %.1d; %.1d; %.1d", empfang[0], empfang[1], empfang[2], empfang[3], empfang[4], empfang[5], empfang[6], empfang[7], empfang[8], empfang[9], empfang[10]);   



                              //            printf("255-empfang[7]: %d \n\r", 255-empfang[6]);

                              //            printf("256*(255-empfang[7])): %d \n\r", 256*(255-empfang[6])); 

                              //            printf("empfang[8]: %d \n\r",empfang[7]);



                            // empfangene Daten zu leitwert Decodieren

                                       LW_now = (256*(255-empfang[6]))+empfang[7];

                                      

                                       printf("Ausgabe LW_now  %d \n\r",LW_now);

                                      

                                       if (LW_now < LW +30  )

                                       {

                                                   LW = LW_now;

                                                   LWF = LW_now*1.0;

                                       }

                                       printf ("Log Aktiv Aktueller Leitwert: %.1f \n\r", LWF);

                             }

                             // Sendestring ändern (Temp Anfrage)

                                    send[0] = 0xFD;

                                      send[1] = 0x00;

                                      send[2] = 0x02;                            





                                      break;

                                      case 0xFD:

                                                                     //printf("\r\nLW: ;0=%.1d; 1=%.1d; 2=%.1d; 3=%.1d; 4=%.1d; 5=%.1d; \n\r 6=%.1d; 7=%.1d; 8=%.1d; 9=%.1d; 10=%.1d; 11=%.1d;%X; %X;", empfang[0], empfang[1], empfang[2], empfang[3], empfang[4], empfang[5], empfang[6], empfang[7],empfang[8],empfang[9],empfang[10],empfang[11],send[0], send[2]);                                           

                                                                     //printf("\r\nLW: ;0=%.1d; 1=%.1d; 2=%.1d; 3=%.1d; 4=%.1d; 5=%.1d; 6=%.1d; 7=%.1d; 8=%.1d; 9=%.1d;%X; %X;", empfang[0], empfang[1], empfang[2], empfang[3], empfang[4], empfang[5], empfang[6], empfang[7],empfang[8],empfang[9],send[0], send[2]);

                                                      if(empfang[0] == 253 && (empfang[1] == 5 || empfang[1] == 3) && (empfang[2] == 25 || empfang[2] == 11) && (empfang[3] == 121 || empfang[3] == 183|| empfang[3] == 182 || empfang[3] == 120) && send[0] == 0xFD && send[2] == 0x02)

                                                      {

                                                                     //printf("Ausgabe Step 1 %d  %d \r\n",empfang[6], empfang[7]);

                                    // empfangene Daten zu Temp Decodieren

                                               Temp = (256*(255-empfang[6]))+empfang[7];

                                               TempF = Temp*0.1;

                                               // Sendestring ändern (LW Anfrage)

      



                                                                                                                                             // Consolenzeile "löschen"k

                                    for( int i=0; i<60; ++i )

                                                                                                                                  {

                                                                                                                                    putchar( '\b' );

                                                                                                                                  }

                                                                                                                                                              

                                               // Consolenausgabe

                                               printf ("Log Aktiv, Aktuelle Temperatur:  %.1f C \n\r", TempF);

                                               //printf ("\r\nWerte: %.1d, %.1d, %.1d, %.1d, %.1d, %.1d, %.1d, %.1d, %.1d, %.1d, %.1d", empfang[0], empfang[1], empfang[2], empfang[3], empfang[4], empfang[5], empfang[6], empfang[7], empfang[8], empfang[9], empfang[10]);

                                             //fprintf(fp,"\r\nLog Aktiv, Aktueller Leitwert:; ;%.1f ;;bei;;; ;%.1f ;gradC", LWF, TempF);

                                             //fprintf(fp,"\r\nTemp: ;%.1d; %.1d; %.1d; %.1d; %.1d; %.1d; %.1d; %.1d; %.1d; %.1d; %.1d", empfang[0], empfang[1], empfang[2], empfang[3], empfang[4], empfang[5], empfang[6], empfang[7], empfang[8], empfang[9], empfang[10]);   

             

                                                                                                                            // Wert in Serverfile schreiben                                                                                                                                                                                                                                                                            

                                             std::ofstream outFile(Zielpfad);

                                             outFile << LW << 'L' << Temp << 'T' <<std::endl;

                                             outFile.close();



                                             // simple Logfile erstellen .............................................

                                             if(simple_log != 1){

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    

                                                 time_t Zeitstempel;

                                                 tm *nun;

                                                 Zeitstempel = time(0);

                                                 nun = localtime(&Zeitstempel);

                                                            

                                                             char LWSTR[5];

                                                             float_to_string(LWF,LWSTR);

                                                             char TempSTR[5];

                                                             float_to_string(TempF,TempSTR);

                                                                                                                                                                          

                                                             char *c;

                                                             while((c = strchr(LWSTR, '.'))!=NULL)*c = ',';

                                                             while((c = strchr(TempSTR, '.'))!=NULL)*c = ',';          

                                                             if(empfang[1] == 5){

                                                                             printf("\n\rGreisinger GMH3431 erkannt LW: %.2f Temp:%.2f\n\r",LWF *0.1,TempF);

                                                             }                                                                                                                           

                                                             if(empfang[1] == 3){

                                                                             printf("\n\r**************************************************************************\n\r");

                                                                             printf("Greisinger GMH3410 erkannt es wird nur das Geraet GMH 3431 unterstuetzt!\n\r");

                                                                             printf("**************************************************************************\n\r");

                                                                             //exit(0);

                                                                            

                                                             }                                                                                                                                                                                                                                                                                                                        

                                                             //printf("\n\rLW: %.2f Temp:%.2f\n\r",LWF,TempF);

                                                             std::ofstream outFile2(Slogpfad, std::ios::app );           

                                                             outFile2 << nun->tm_mday << "." << nun->tm_mon+1 << "." << nun->tm_year+1900 << " " << nun->tm_hour << ":" << nun->tm_min << ":" << nun->tm_sec << ";" << LWSTR << ";" << TempSTR << std::endl;

                                                             outFile2.close();                                                                                                                                                                           

                                             }

                                              //..............................................................................

                                                            

                                                                                                                           

                                             // 900 ms warten           

                                                            

                                              }

                                              send[0] = 0xFE;

                                  send[1] = 0x00;

                                  send[2] = 0x3D;

                                              break;

                                             

                                              default:

                                                             send[0] = 0xFD;

                                      send[1] = 0x00;

                                      send[2] = 0x02; 

                                              break;

             }

                                                                                                                                                              
                                    // Sendeanfrage erneut senden

                                    WriteFile (hCom, &send, 3, &iBytesWritten, NULL); // Senden der Bytes

                                            Sleep(900);       

                               }

                       }


                       if (dwEvtMask & EV_ERR)

                       {

                               MessageBox (NULL, "Error empfangen", "Error: ReadFile", MB_OK);

                               break; // Schleifen-Abbruch

                       }

               }

        }

        while (1);


        CloseHandle (hCom);     // COM schließen

        CloseHandle (o.hEvent); // Event-Handle zurückgeben


        return (0);
}


