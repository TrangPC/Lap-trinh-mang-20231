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
#include <pthread.h>
#include <malloc.h>

#include"auctionserver.h"

MyUser lstUsers[MAX_CLIENT];
Room* lstRooms[MAX_ROOM];

// ItemList lstItem[2];
// BiddingList bidd1List, bidd2List;
// BroadcastList blist;
// SellList sellList[10];
int iBcount = 0;
int dem;

void Phase_1();
int ReadRegistrationFile(char *pszFileName);
int AuthenticateUser(int iSock, struct MyUser* client);
int ReadItemFile(char *pszFileName);
int WriteRoomItem();
int iWelSock;
socklen_t client_addr_len;

char szAucServIP[20];

int g_client_socket[MAX_CLIENT] = { 0 };

void* client_thread(void* arg)
{
    struct MyUser* args = (struct MyUser*)arg;
    //printf("New thread! %d, ip: %s.\n", args->c,inet_ntoa(args->addr.sin_addr));
    int auth = AuthenticateUser(args->c, args);
    int dem = 5;
    while(dem > 0 && auth != 1){
        dem--;
        int auth = AuthenticateUser(args->c, args);
        char* reject = "User account or password incorrect\n";
        send(args->c, reject, strlen(reject), 0);
    }

    if(dem > 0 && auth == 1){
        printf("Infor: %s, %s, %s\n", args->szUserName, args->szUserPass, args->szUserRole);

        if(strcmp(args->szUserRole, "1")==0){
            printf("Bidder Login!\n");
            backToListRoom: char mess[1024];
            int imess;
            strcat(mess, "List Room: \n");
            for(int i=0;i<MAX_ROOM;i++){
                if(lstRooms[i]->id>=0){
                    strcat(mess,"Room_id: ");
                    sprintf(mess + strlen(mess), "%d" , lstRooms[i]->id);
                    strcat(mess," - Seller: ");
                    strcat(mess, lstRooms[i]->seller->szUserName);
                    strcat(mess, ".\n");
                }
            }
            send(args->c, &mess, strlen(mess), 0);

            imess = recv(args->c, &mess, 1024, 0);
            if(imess<=0){
                perror("Recv");
            }
            int id_room;
            sscanf(mess,"%d", &id_room);
            
        


            //printf("seller: %s", lstRooms[id_room]->seller->szUserName);
            //strcat(notification,lstRooms[id_room]->seller->szUserName);
            //strcat(notification,".\n");

            char* requestJoinRoom;
            sprintf(requestJoinRoom, "%d join room. Y/N?", args->c);

            send(lstRooms[id_room]->seller->c,requestJoinRoom,strlen(requestJoinRoom), 0);
            
            imess = recv(lstRooms[id_room]->seller->c, mess, 1024, 0);
            if (imess<=0)
            {
                perror("Recv");
                goto backToListRoom;
            }
            char response = mess[0];
            if (sscanf(mess, "%c %d", &response, &port) == 2 && (response == 'N'||response == 'n'))
            {
                perror("Reject"); 
            }
            if (sscanf(mess, "%c %d", &response, &port) == 2 && (response =='Y'|| response == 'y'))
            {
                if(lstRooms[id_room]->numClient<MAX_CLIENT_ROOM){
                lstRooms[id_room]->list_client[lstRooms[id_room]->numClient] = args;
                lstRooms[id_room]->numClient++;
            }
                char* notification = "Joined to room. \n";
                send(port, notification, strlen(notification), 0);
                char* notification2 = "A bidder join room\n";
                // strcat(notification, args->szUserName);
                // strcat(notification,".\n");
                send(lstRooms[id_room]->seller->c, notification2, strlen(notification2), 0);
            
            //printf("Res: %d, numcli: %d, seller: %s\n", id_room, lstRooms[id_room]->numClient,lstRooms[id_room]->seller->szUserName );


                while( 0==0){
                        printf("receive mess \n");
                        imess = 0;
                        imess = recv(args->c, mess, 1024, 0);
                        if(imess<=0){
                            perror("Recv");
                        }
                        send(lstRooms[id_room]->seller->c, mess, strlen(mess), 0);

                        printf("Res: %d, numcli: %d\n", id_room, lstRooms[id_room]->numClient);
                        for(int i=0;i<lstRooms[id_room]->numClient;i++){
                            if(lstRooms[id_room]->list_client[i]->c!=args->c){
                                send(lstRooms[id_room]->list_client[i]->c, mess, strlen(mess), 0);
                            }
                            //sendto(iWelSock, mess, imess, 0, (struct sockaddr*)room->list_client[i], sizeof(room->list_client[i]) );
                        }       
                }
            }
           


        

        }else if(strcmp(args->szUserRole, "2")==0){
            char mess[1024];
            int imess;
            printf("Seller Login!\n");
            int dem = 5;
            int repeat = 0;
            printf("sock: %d, %s\n", args->c, args->szUserName);
            while(dem>0 && repeat == 0){
                dem--;
                char* reject = "Create a Room? (Y/n)\n";
                printf("sock1: %d, %s\n", args->c, args->szUserName);

                send(args->c, reject, strlen(reject), 0);

                printf("sock2: %d, %s\n", args->c, args->szUserName);

                imess = recv(args->c, mess, 1024, 0);
                if(imess<=0){
                    perror("Recv confirm");
                    //exit(1);
                }
                //mess[imess] = '\0';
                char response = mess[0];
                //sscanf(mess,"%s", response);
                printf("mess: %c .", response);
                    
                if((response=='Y' || response=='y')&&(imess==2)){
                    //printf("Response: %s\n", mess);
                    repeat = 1;
                    int kt = 0;
                    int id_room = -1;
                    for(int i=0;i<MAX_ROOM;i++){
                        if(lstRooms[i]->id<0){
                            args->room_id = i;
                            lstRooms[i]->id=i;
                            lstRooms[i]->seller = args;
                            id_room = i;
                            kt = 1;
                            break;
                        }
                    }
                    if(kt == 1){
                        printf("create a Room\n");

                        while( 0==0){
                            printf("receive mess \n");
                            //mess = { 0 };
                            imess = 0;
                            imess = recv(args->c, mess, 1024, 0);
                            if(imess<=0){
                                perror("Recv");                                                                                                            
                            }
                            // if (strcmp (mess, "New bidder wants to join room. Y/N?"))
                            // {
                            //     memset(mess,'\0',strlen(mess));
                            //     recv(args->c, mess, 1024, 0);
                            // }


                            printf("Res: %d, numcli: %d\n", id_room, lstRooms[id_room]->numClient);
                            for(int i=0;i<lstRooms[id_room]->numClient;i++){
                                send(lstRooms[id_room]->list_client[i]->c, mess, strlen(mess), 0);
                                //sendto(iWelSock, mess, imess, 0, (struct sockaddr*)room->list_client[i], sizeof(room->list_client[i]) );
                            }
                        }
                    // tao vat pham 
                    // mua vat pham

                    B: char *getItem = "1. Them vat pham moi\n2. Chon vat pham\n";
                    send(args->c, getItem, strlen(getItem), 0);
                    imess = 0;
                    imess = recv (args->c, mess, 1024, 0);
                    if (imess <=0)
                        perror("Recv");
                    char response = mess[0];
                    Item* item;
                    if (imess == 2 && response =='1')
                    {
                        sscanf(mess,"%d %s %ld %ld %d %d", &item->id, &item->name, &item->startTime, &item->endTime, &item->targetPrice);
                        // luc thiet lap status khong
                        &item->startTime = 0; 


                        int add = WriteItemFile("Item.txt", item);
                        mess = "Add new Item successfully.\n";
                        reject = "Add new Item fail\n";
                        if (add == 1) send(args->c, mess, strlen(mess), 0);
                        else send(args->c, reject, strlen(reject),0);
                    }
                    if (item == 2 && response == '2')
                    {
                        // ReadItemFile("Item.txt");
                        mess = "";
                        char line[256];
                        FILE *fp;
                        fp = open("Item.txt","r");
                        Item items[100];
                        int itemCount = 0;
                        if (fp == NULL) perror("Can't open file\n");
                        while(fscanf(fp, "%s %ld %ld %d %d %d", items[itemCount].name, items[itemCount].startTime, items[itemCount].endTime, items[itemCount].price, items[itemCount].targetPrice,items[itemCount].status) ==6)
                        {
                            items[id].id=itemCount;
                            sprintf(mess + strlen(mess), "ID: %d - ", items[itemCount].id);
                            sprintf(mess + strlen(mess), "Name: %s - ", items[itemCount].name);
                            sprintf(mess + strlen(mess), "Price: %d - ", items[itemCount].price);
                            sprintf(mess + strlen(mess), "TargetPrice: %d - ", items[itemCount].targetPrice);
                            sprintf(mess + strlen(mess), "Status: %d\n", items[itemCount].status);

                            itemCount++;
                        }
                        send(args->c, mess, strlen(mess),0);
                        C: char session = "Tao phien dau gia. Y?N";
                        send(args->c, session, strlen(session),0);
                        imess = recv(args->c,mess,1024,0);
                        if (imess<=0)
                        perror("Recv");
                        char response = mess[0];
                        if (imess==2&&(response=='Y'||response=='y'))
                        {
                            mess = "Them vat pham: ";
                            send(args->c, mess, strnlen(mess), 0);
                            imess = recv(args->c, mess, 1024, 0);
                            if (imess<=0)
                            perror("Recv");
                            goto C;
                            int item_id;
                            sscanf(mess,"Id: ", item_id);
                            WriteRoomItem();


                        }
                        if (imess==2&&(response=='N'||response=='n'))
                        {
                            goto B;
                        }

                    }
                    

                    }else{
                        repeat = 0;
                        printf("Full Room\n");
                    }
                }else{
                    printf("res: %d, %d", strcmp(mess, "Y"),strcmp(mess, "y"));
                }    
            }
            

            // buffRx[iBuffRx] = '\0';
        
            // sscanf(buffRx,"%s %s %s", szStatus, szUserName, szUserPass);
            
        }    

    } else {
        char* reject = "Log in incorrectly more than 5 times!\n";
        send(args->c, reject, strlen(reject), 0);
    }

    

    free(arg);
    arg = NULL;                  
    return NULL;

    // int c = *((int*)arg);
    // free(arg);
    // arg = NULL;
    // AuthenticateUser(c, "127.0.0.1");
    // return NULL;
}

int main()
{                     
        //Begin of Phase 1
               // ReadRegistrationFile("Registration.txt");
        Phase_1();
        printf("\nEnd of Phase 1 for Auction Server");
        //End of Phase 1
        

}

void Phase_1()
{
    for(int i=0;i<MAX_ROOM;i++){
        struct Room* room = (struct Room*)malloc(sizeof(struct Room));
        room->id = -1;
        room->numClient = 0;
        lstRooms[i] = room;
    }
        sprintf(requestJoinRoom, "%d join room. Y/N?", args->c);("Registration.txt");
        int c, bytes_recieved , true = 1;  
        char send_data [1024] , recv_data[1024];

        struct sockaddr_in addAucServer, addClient;    
        int sin_size;
        
        int iFlagAddInfo;
        struct addrinfo Hints, *Serverinfo;
        int iSockOpt = 1;
        
        int iAucPort = 1933;                        
        
        //Read Registration File and populate the Users with their data
        
        
        memset(&Hints, 0, sizeof(Hints));
        Hints.ai_family = AF_INET;
        Hints.ai_socktype = SOCK_STREAM;        
        
        if((iFlagAddInfo = getaddrinfo("localhost", "1933", &Hints, &Serverinfo)) != 0)
        {
            perror("getaddrinfo ");            
        }
        
        void *pAddr;
        struct sockaddr_in *pIP = (struct sockaddr_in *) Serverinfo->ai_addr;
        
        pAddr = &pIP->sin_addr;
        inet_ntop(Serverinfo->ai_family, pAddr, szAucServIP, sizeof(szAucServIP));                        
        
        //Create Welcoming Socket
        if ((iWelSock = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        {
            perror("Socket");
            exit(1);
        }             
        
        if (setsockopt(iWelSock, SOL_SOCKET,SO_REUSEADDR, &iSockOpt, sizeof(int)) == -1) 
        {
            perror("Setsockopt");
            exit(1);        
        }
        struct hostent *host;
        host = gethostbyname("localhost");
        
        // Configuring the Welcoming Socket
        memset(&addAucServer, 0 ,sizeof(addAucServer));
        addAucServer.sin_family = AF_INET;         
        addAucServer.sin_port = htons(iAucPort);     
        addAucServer.sin_addr = *((struct in_addr *)host->h_addr);                    

        if (bind(iWelSock, (struct sockaddr *)&addAucServer, sizeof(struct sockaddr)) == -1) 
        {
            perror("Unable to bind");
            exit(1);
        }

        if (listen(iWelSock, 10) == -1) 
        {
            perror("Listen");
            exit(1);
        }
		
        printf("\nPhase 1: Auction Server has TCP port number %d and IP address %s" , iAucPort, szAucServIP);
        fflush(stdout);
        int i = 0;
        
        while(0==0)
        {  
                sin_size = sizeof(struct sockaddr_in);

                c = accept(iWelSock, (struct sockaddr *)&addClient, &sin_size);
                //printf("New connection!\n");

                int i = 0;
                for (i = 0;i < MAX_CLIENT;i++)
                {
                    if (g_client_socket[i] == 0)
                    {
                        g_client_socket[i] = c;
                        break;
                    }
                }
                if (i < MAX_CLIENT)
                {
                    
                    pthread_t id = 0;
                    struct MyUser* args = (struct MyUser*)malloc(sizeof(struct MyUser));
                    args->c = c;
                    args->addr = addClient;
                    //printf("Add an client! %d -- %d \n", c, args->c);
                    

                    // int* arg = (int*)calloc(1, sizeof(int));
                    // *arg = c;

                    pthread_create(&id, NULL, client_thread, (void*)args);
                }else
                {
                    char* reject = "Room is full now, please try again later!\n";
                    send(c, reject, strlen(reject), 0);
                }
                
                //if(!fork())
                //{
                        //close(iWelSock);
                       
                //}
               // close(iNewSock);
                fflush(stdout);
        }
                
        close(iWelSock);
}

int ReadRegistrationFile(char *pszFileName)
{
    FILE *fp;
    char szName[20], szPassword[20], szRole[20];    
    
    fp = fopen(pszFileName,"r");
    
    if(fp == NULL)
    {
        printf("\nRegistration File reading error!");
        exit(1);        
    }
    
    int i = 0;
    
    while( !feof(fp) )
    {
        fscanf(fp,"%s %s %s", szName, szPassword, szRole);
        
        
        strcpy(lstUsers[i].szUserName, szName);
        strcpy(lstUsers[i].szUserPass, szPassword);
        strcpy(lstUsers[i].szUserRole, szRole);   
        //printf("add: %s, %s, %s \n", lstUsers[i].szUserName, lstUsers[i].szUserPass, lstUsers[i].szUserRole);    
        
        i++;
    }
    dem=i;
    //printf("\nRegistration file reading complete.");
    
    fclose(fp);
    
    return 0;
}
int WriteRegistrationFile(char *pszFileName)
{
    FILE *fp;
    char szName[20], szPassword[20], szRole[20];    
    
    fp = fopen(pszFileName,"w");
    
    int i = 0;
    
    while( i < dem )
    {
        strcpy(szName, lstUsers[i].szUserName);
        strcpy(szPassword,lstUsers[i].szUserPass );
        strcpy( szRole,lstUsers[i].szUserRole);   
        fprintf(fp,"%s %s %s\n", szName, szPassword, szRole);       
        i++;
    }
    
    //printf("\nRegistration file reading complete.");
    
    fclose(fp);
    
    return 0;
}
int WriteItemFile(char *pszFileName, Item *item)
{
    FILE *fp;
    // char szName[20];
    // int szPrice;
    // time_t szStartTime, szEndTime;
    // int szPrice, szTargetPrice, szstatus;

    fp = fopen(pszFileName, "ab");
    if (fp == NULL) perror ("Can't open Item file\n");
    else 
    {
    fwrite(item, sizeof(item), 1, fp);
    return 1;
    fclose(fp);
    }
}
int DeleteItem(char *pszFileItem, char name)
{
    FILE *fp;
        fp = fopen(pszFileName, "r+b");
        if (fp == NULL) perror("Can't open Item file\n");
        Item curr;
        size_t itemSize = sizeof(Item);
        int flag = 0;
        while (fread(&curr, itemSize,1,fp)==1)
            {fseek (fp, -itemSize, SEEK_CUR);
            fwrite("",1,itemSize,fp);
            flag = 1;
            break;
            return 1; 
            }
    if (flag == 0)
    {
        return 0;
    }
}

int AuthenticateUser(int iSock, struct MyUser* client)
{
        char buffRx[1024], buffTx[1024];
        int iBuffRx, iBuffTx;
        char szStatus[10];
        char szUserName[20];
        char szUserPass[20];
        char szRole[20];
        int  i = 0;        
    
        iBuffRx = recv(iSock, buffRx, 1024, 0);
        if(iBuffRx<=0){
            perror("Recv");
            //exit(1);
        }
        printf("sock: %d, buffer: %s\n", iSock, buffRx);
        buffRx[iBuffRx] = '\0';
        
        sscanf(buffRx,"%s %s %s", szStatus, szUserName, szUserPass);

        //printf("Infor %s, %s, %s . %d \n", szStatus, szUserName, szUserPass, strcmp(szStatus,"Login"));

        if(strcmp(szStatus,"Login") == 0)
        {
                printf("Phase 1: Authentication request. \n");
                printf("User#: %s. Password: %s. User IP Addr: %s.\n", szUserName, szUserPass, inet_ntoa(client->addr.sin_addr) );
                
                for(i = 0;i<dem;i++)
                {
                        if(strcmp(szUserName, lstUsers[i].szUserName) == 0)
                        {
                                if(strcmp(szUserPass, lstUsers[i].szUserPass) == 0 )
                                {
                                        //lstUsers[i].iType = iType;
                                        // strcpy(lstUsers[i].szUserIP, pszIP);

                                        strcpy(client->szUserName, szUserName);
                                        strcpy(client->szUserPass, szUserPass);
                                        strcpy(client->szUserRole, lstUsers[i].szUserRole);
                                        //printf("LOGININF: %s, %s, %s \n", lstUsers[i].szUserName, lstUsers[i].szUserPass, lstUsers[i].szUserRole);    

                    
                                        strcpy(buffTx,"Login OK!\n");                                                                            
                                        
                                        printf(" Authorized: Yes\n");
                                        
                                       
                                        
                                        iBuffTx = send(iSock, buffTx, strlen(buffTx), 0);
                                        fflush(stdout);
                                        return 1;
                                }
                                else
                                {
                                        strcpy(buffTx,"Rejected#: User account or password incorrect\n");
                                        iBuffTx = send(iSock, buffTx, strlen(buffTx), 0);
                                        printf(" Authorized: No\n");         
                                        return 0;                               
                                }
                        }
                }                
        }
        else
         if(strcmp(szStatus,"Signup") == 0)
         {
             sscanf(buffRx,"%s %s %s %s", szStatus, szUserName, szUserPass,szRole);
             int kt=0;
             for(i = 0;i<dem;i++)
                        if(strcmp(szUserName, lstUsers[i].szUserName) == 0)
                        {
                               kt=1;
                               break; 
                        }
                        
               if(kt==0)
                {
                    strcpy(lstUsers[dem].szUserName,szUserName);
                    strcpy(lstUsers[dem].szUserPass,szUserPass);
                    strcpy(lstUsers[dem].szUserRole,szRole);
                    dem++;
                    WriteRegistrationFile("Registration.txt");
                    strcpy(buffTx,"Success#");
                    return 2;
                } 
                else
                {
                    strcpy(buffTx,"Faild# TK da ton tai");
                    return 0;
                }
                iBuffTx = send(iSock, buffTx, strlen(buffTx), 0);

         }
        else
        {
                strcpy(buffTx,"Bad#");
                iBuffTx = send(iSock, buffTx, strlen(buffTx), 0);
        }
        
        return 0;
}
WriteRoomItem(){
    
}
int ReadItemFile(char *pszFileName)
{
    File *fp;
    char sz
}