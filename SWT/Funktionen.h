#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>
#include <sstream>

 



void requestComport(char *comport){

	printf ("Greisinger GMH 3400 einschalten und Datenkabel Verbinden");
    printf ("\r\nPort angeben (BsP.:COM1): ");
    scanf ("%5s",&comport);
    printf ("\r\nPort angeben (BsP.:COM1): ");
}

void requestIP(char *IPEndstellen){    
    printf ("\r\nGeraete IP Enstellen eingeben (BsP.: 65): ");
    scanf ("%5s",&IPEndstellen);
    printf ("\r\n"); 
  
}





inline unsigned int countdigits(unsigned int x)

{
    unsigned count=1;

    unsigned int value= 10;

    while (x>=value)

    {

        value*=10;

        count++;

    }

    return count;

}

 


/** Number on countu**/

 

int n_tu(int number, int count)

{

  int result=1;

  while(count-- > 0)

  result *=number; 

 

  return result;

  }

 

 

/***Convert float to string***/

void float_to_string(float f, char r[])

{

long long int length, length2, i, number, position, sign;

float number2;

 

sign=-1;   // -1 == positive number

if (f <0)

{

sign='-';

f *= -1;   

}  

 

 

number2=f; 

number=f;

length=0;  // size of decimal part

length2=0; //  size of tenth

 

 

/* calculate length2 tenth part*/

while( (number2 - (float)number) != 0.0 && !((number2 - (float)number) < 0.0) )

{

 

number2= f * (n_tu(10.0,length2+1));

number=number2;

 

length2++;

 

}

 

/* calculate length decimal part*/

for(length=(f> 1) ? 0 : 1; f > 1; length++)

  f /= 10;

 

 

position=length;

length=length+1+length2;

number=number2;

if(sign=='-')

{

length++;

position++;

}

 

for(i=length; i >= 0 ; i--)

{

if(i== (length))

  r[i]='\0';

else if(i==(position))

  r[i]='.';

else if(sign=='-' && i==0)

  r[i]='-';

else 

{

  r[i]= (number % 10)+'0';

  number /=10;

}

}

}

 
