#ifndef _SKEL_PRIVATE_H
#define _SKEL_PRIVATE_H

#include "table_server.h"
#include "distributed_database.h"
#include "sdmessage.pb-c.h"


// helpers to perform an action over a table
// verifying if the message is valid
// performing action and updating msg with regard to its result
int error(MessageT* msg);
int put(MessageT* msg, struct TableServerDistributedDatabase* ddb);
int get(MessageT* msg, struct TableServerDistributedDatabase* ddb);
int del(MessageT* msg, struct TableServerDistributedDatabase* ddb);
int size(MessageT* msg, struct TableServerDistributedDatabase* ddb);
int getkeys(MessageT* msg, struct TableServerDistributedDatabase* ddb);
int gettable(MessageT* msg, struct TableServerDistributedDatabase* ddb);
int stats(MessageT* msg, struct TableServerDistributedDatabase* ddb);


#endif