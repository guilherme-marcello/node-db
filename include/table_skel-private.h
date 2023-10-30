#ifndef _SKEL_PRIVATE_H
#define _SKEL_PRIVATE_H

#include "table.h"
#include "sdmessage.pb-c.h"


// helpers to perform an action over a table
// verifying if the message is valid
// performing action and updating msg with regard to its result
int error(MessageT* msg);
int put(MessageT* msg, struct table_t* table);
int get(MessageT* msg, struct table_t* table);
int del(MessageT* msg, struct table_t* table);
int size(MessageT* msg, struct table_t* table);
int getkeys(MessageT* msg, struct table_t* table);
int gettable(MessageT* msg, struct table_t* table);


#endif