#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include"client.h"

int errno;

Client client1;
void ReadSeller(char *pszSellerFile);

void Phase_1();
void ServerResponse_Login(char *pszServRes);

int iPreAucPort;
char szPreAucIP[20];
void chuphong()
{
 
   printf(" Xin chao chu phong. Vui long chon\n");
   printf("1. Them san pham\n");
   
}
void nguoidaugia()
{
   printf("\nXin chao nguoi dau gia\n");
   printf("1. Xem danh sach phong\n");
}
int main(int argc,char* argv[]) 
{        
        //ReadSeller("mau1.txt");
        strcpy(client1.szClientStatus,argv[1]);
        strcpy(client1.szClientName,argv[2]);
        strcpy(client1.szClientPass,argv[3]);
        if(strcmp(client1.szClientStatus,"Signup")==0)

             strcpy(client1.szClientRole,argv[4]);
        //Begin of Phase 1
        Phase_1();
        printf("\nEnd of Phase 1 for %s", client1.szClientName);
        //End of Phase 1
       
        return 0;
}

void ReadSeller(char *pszSellerFile)
{
    FILE *fp;    
    
    fp = fopen(pszSellerFile,"r");
    
    if(fp == NULL)
    {
        printf("\nSeller 1 File reading error!");
        exit(1);        
    }
    
    int i = 0;
    
    while( i < 1 )
    {
        fscanf(fp,"%d %s %s", &client1.iType, client1.szClientName, client1.szClientPass);                
        i++;
    }
    
    //printf("\nSeller 1 Read Complete");
    
    fclose(fp);        
}

void Phase_1()
{
        int iCLSock, bytes_recieved;  
        char recv_data[1024];
        struct hostent *host;
        struct sockaddr_in addrAucServ, addrSell;          
        socklen_t addrLen = sizeof(addrSell);
        char szStatus[200];
        
        host = gethostbyname("localhost");
        
        //Phase 1
        if ((iCLSock = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        {
            perror("Socket");
            exit(1);
        }
        
        memset(&addrAucServ, 0, sizeof(addrAucServ));
        addrAucServ.sin_family = AF_INET;     
        addrAucServ.sin_port = htons(1933);   
        addrAucServ.sin_addr = *((struct in_addr *)host->h_addr);

        memset(&addrSell, 0, sizeof(addrSell));
        addrSell.sin_family = AF_INET;
        addrSell.sin_port = htons(0);
        addrSell.sin_addr.s_addr = inet_addr("127.0.0.1");
        
        if(bind(iCLSock, (struct sockaddr *) &addrSell, sizeof(addrSell)) == -1)
        {
            perror("Bind:");
            exit(1);
        }               
                
        if(getsockname(iCLSock, (struct sockaddr *) &addrSell, &addrLen) != 0)
        {
            perror("GetSockName:");
            exit(1);
        }
        
        printf("\nPhase 1: Seller1 has TCP Port %d  and IP address %s", ntohs(addrSell.sin_port), inet_ntoa(addrSell.sin_addr));
        fflush(stdout);
                
        if (connect(iCLSock, (struct sockaddr *)&addrAucServ, sizeof(struct sockaddr)) == -1) 
        {
            perror("Connect LOG: ");
            exit(1);
        }
        
        strcpy(szStatus, client1.szClientStatus);
        strcat(szStatus, " ");
        strcat(szStatus, client1.szClientName);
        strcat(szStatus, " ");
        strcat(szStatus, client1.szClientPass);
        if(strcmp(client1.szClientStatus,"Signup")==0)
         {
            strcat(szStatus, " ");
            strcat(szStatus, client1.szClientRole);
         }   
        
        printf("\nPhase 1: Login request. User: %s password: %s", client1.szClientName, client1.szClientPass);
        
        send(iCLSock, szStatus, strlen(szStatus), 0);               
        
        bytes_recieved = recv(iCLSock, recv_data,1024,0);
        recv_data[bytes_recieved] = '\0';        
        ServerResponse_Login(recv_data);    
        fflush(stdout);    
        
        close(iCLSock);
}

void ServerResponse_Login(char *pszServRes)
{    
    char s[2] = " ";
    char *pToken;
    
    pToken = strtok(pszServRes, s);
    
    if(strcmp(pToken, "1") == 0)
    {
        
       // printf("\nXin chao Nguoi dau gia\n");
       nguoidaugia();
        //printf("\nPhase 1: Auction Server has IP Address %s and PreAuction TCP Port  Number %d", szPreAucIP, iPreAucPort);
    }
    else if(strcmp(pToken, "2") == 0)
    {
       // printf("\nXin chao Chu phong");
       chuphong();
    }
    else if(strcmp(pToken, "Rejected#") == 0)
    {
        printf("\nPhase 1: Login request reply: Rejected");
    }
    else if(strcmp(pToken, "Success#") == 0)
    {
      if(strcpy(client1.szClientRole,"1")==0)
         nguoidaugia();
      else chuphong();
    }
    else if(strcmp(pToken, "Faild#") == 0)
    {
      printf("\n Tai khoan da ton tai\n");
    }
    else
    {
        printf("\nBad Command from Auction Server");
    } 

}

