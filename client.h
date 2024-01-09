
#ifndef CLIENT_H
#define	CLIENT_H

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct Client
    {
        char szServerIP[20];
        char szPortPhase3[10];
        char szPortPhase2[10];
        char szClientName[20];
        char szClientPass[20];
        char szClientRole[20];
        char szClientStatus[20];
        int iType;
    } Client;        


#ifdef	__cplusplus
}
#endif

#endif	

