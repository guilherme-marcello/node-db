#ifndef _SKEL_PRIVATE_H
#define _SKEL_PRIVATE_H

#include "table_server.h"
#include "sdmessage.pb-c.h"


// helpers to perform an action over a table
// verifying if the message is valid
// performing action and updating msg with regard to its result
int error(MessageT* msg);
int put(MessageT* msg, struct TableServerDatabase* db);
int get(MessageT* msg, struct TableServerDatabase* db);
int del(MessageT* msg, struct TableServerDatabase* db);
int size(MessageT* msg, struct TableServerDatabase* db);
int getkeys(MessageT* msg, struct TableServerDatabase* db);
int gettable(MessageT* msg, struct TableServerDatabase* db);


#endif