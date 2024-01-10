
#ifndef AUCTIONSERVER_H
#define	AUCTIONSERVER_H

#include <netinet/in.h>

#define MAX_CLIENT 1024
#define MAX_ROOM 16
#define MAX_CLIENT_ROOM 128

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct MyUser {
        int c;
        int room_id;
        char szUserName[20];
        char szUserPass[20];
        char szUserRole[20];
        char szUserIP[16];
        struct sockaddr_in addr;
    } MyUser;

    typedef struct Room {
        int id;
        int numClient;
        struct MyUser* list_client[MAX_CLIENT_ROOM];
        struct MyUser* seller;
    } Room;

    typedef struct Item{
        int id;
        char name[100];
        time_t startTime;
        time_t endTime;
        int price;
        int targetPrice;
        int status;
    } Item;
    
    // typedef struct ItemList
    // {
    //     char szUser[20];
    //     char szItem[6][20];
    //     char szPrice[6][20];
    // } ItemList;
    
    // typedef struct BiddingList
    // {
    //     char szBidder[20];
    //     char szItems[10][20];
    //     char szSellers[10][20];
    //     char szPrice[10][20];
    // } BiddingList;
    
    // typedef struct BroadcastList
    // {
    //     char szSeller[15][20];
    //     char szItem[15][20];
    //     char szPrice[15][20];
    // } BroadcastList;
    
    // typedef struct SellList
    // {
    //     char szBidder[20];
    //     char szSeller[20];
    //     char szItem[20];
    //     int iPrice;
    //     int iProfit;
    // } SellList;

#ifdef	__cplusplus
}
#endif

#endif	/* AUCTIONSERVER_H */

