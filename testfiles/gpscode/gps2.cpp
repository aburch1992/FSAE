#include "gps.h"

//Its a good idea to have a debug function
FILE *stream;
int nFd = 0;
struct termios stNew;
struct termios stOld;
//Open Port & Set Port
int SerialInit(){
	nFd = open(DEVICE, O_RDWR|O_NOCTTY|O_NDELAY);
	if(-1 == nFd){
		perror("Open Serial Port Error!\n");
		return -1;
	}
	if( (fcntl(nFd, F_SETFL, 0)) < 0 ){
		perror("Fcntl F_SETFL Error!\n");
		return -1;
	}
	if(tcgetattr(nFd, &stOld) != 0){
		perror("tcgetattr error!\n");
		return -1;
	}
	stNew = stOld;
	cfmakeraw(&stNew);//raw mode
	//set speed
	cfsetispeed(&stNew, BAUDRATE);//from header file, baudrate  = 9600
	cfsetospeed(&stNew, BAUDRATE);
	//set databits
	stNew.c_cflag |= (CLOCAL|CREAD);
	stNew.c_cflag &= ~CSIZE;
	stNew.c_cflag |= CS8;
	//set parity
	stNew.c_cflag &= ~PARENB;
	stNew.c_iflag &= ~INPCK;
	//set stopbits
	stNew.c_cflag &= ~CSTOPB;
	stNew.c_cc[VTIME]=0;    //?????????????
	stNew.c_cc[VMIN]=1; //??????????????,??????n*100ms
	//????VTIME=0,???????read()????????
	tcflush(nFd,TCIFLUSH);  //??????????/????????
	if( tcsetattr(nFd,TCSANOW,&stNew) != 0 ){
		perror("tcsetattr Error!\n");
		return -1;
	}
	return nFd;
}
int main(int argc, char **argv){
	char speedbuf[10] = {0};
	char realspeed[10]= {0};	
	char GPSRMC[73]= {0};
	int nRet = 0;
	int nRetw = 0;
	char buf[9][SIZE] = {0};
	int speedinmph = 0;
	if( SerialInit() == -1 ){
		perror("SerialInit Error!\n");
		return -1;
	}

	nRetw = write(nFd, PMTK_SET_BAUD_9600, strlen(PMTK_SET_BAUD_9600));
	printf("nRetw = %d\n", nRetw);
	nRetw = write(nFd, PMTK_SET_NMEA_OUTPUT_RMCONLY, strlen(PMTK_SET_NMEA_OUTPUT_RMCONLY));
        printf("nRetw = %d\n", nRetw);
	nRetw = write(nFd, PMTK_SET_NMEA_UPDATE_10HZ, strlen(PMTK_SET_NMEA_UPDATE_10HZ));
	printf("nRetw = %d\n", nRetw);
	//nRetw = write(nFd, PMTK_API_SET_FIX_CTL_5HZ, strlen(PMTK_API_SET_FIX_CTL_5HZ));
	//printf("nRetw = %d\n", nRetw);
	if(-1 == nRetw){
		perror("Write Data Error!\n");
	}
	for (int j = 0; j<=10000; j++){
		printf("Hey!");
		int k = 0;
		while (k < 9){
			bzero(buf[k],SIZE);
			nRet = read(nFd, buf[k], SIZE);
			if(-1 == nRet){
				perror("Read Data Error!\n");
				break;
			}
			if(0 < nRet){
				if ( buf[k][0] == '$' && buf[k][1] == 'G' && buf[k][2] == 'P' && buf[k][3] == 'R' && buf[k][4] == 'M' && buf[k][5] == 'C'){
					for (int i = 0; i<8; i++){
						buf[0][i] = buf[k][i];
					}
					k = 0;
					printf("Findit!!!");
				}
				printf("buf%d", k);
				printf(" %s\t", buf[k]);
				//fprintf(f, "%s", buf[k]); 
				k++;
				
			}
		}
		if ( strchr(buf[2],'A') != NULL && strchr(buf[8],'*') != NULL){
			bzero(speedbuf,5);
			bzero(realspeed,5);
			printf("Fixed!!!\n");
			bzero(GPSRMC,73);
			for (int i=0; i<9; i++){
				strcat (GPSRMC, buf[i]);//Combine all the buffers
			}			
			printf("%s\n",GPSRMC);
			char* p = GPSRMC;
			for (int i = 0; i<7; i++){
				p = strchr(p+1,',');
			}
			
			for (int i = 0; i<6; i++){
				speedbuf[i] = *(p+i+1);
				if (speedbuf[i] != ','){
					realspeed[i] = speedbuf[i];
				}
			}
			//char *q = strchr(p+1,',');//there should be a better way to do this...
			printf("Current Speed = %s\n", realspeed);
			speedinmph = std::stof(realspeed) * 1.15077945;
			printf("Current Speed (double) = %d\n", speedinmph);
			p = NULL;

			//int speedinknot = stod(realspeed)
			
/* 			printf("Current Speed = %s\n", p);
			printf("Current Speed = %s\n", q); 
			
 			if (',' != *p){
				int time = atoi(p);
				printf("Current Speed = %s\n", p);
			}*/
		}
		
		if ( strchr(buf[2],'V') != NULL ){
			printf("Connecting\n\n");
			speedinmph = -1;
			printf("Current Speed = %d\n", speedinmph);
		}
	}
	
	close(nFd);

	return 0;
}


