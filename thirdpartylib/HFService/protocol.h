#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

/*PROJECT_ID:NW2398*/

#define CODE_ROBOT_VER		"1.7"

extern uint8 *dataOut;
extern int outLen;
void setConfirmed();
void setHeap(void *heap, int size);
void autoResponse();
void upstream(uint8 *data);
void downstream(uint8 *data);



#endif
