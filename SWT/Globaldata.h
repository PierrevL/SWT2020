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
	
	public:  Globaldata(){
		test = 99;	
		simple_log = 0;
		LW = 1000;
		step = 0;
		LWF = 1000.0;
	};	
};

