#ifndef MESSAGE_H_
#define MESSAGE_H_

#define __SVC_0 __svc_indirect(0)

typedef struct msgbuf
{
	int mtype;              /* user defined message type */
	char mtext[1];          /* body of the message */
} MSG_BUF;

typedef struct msg_header {
	struct msg_header *next;
	U32 source_pid;
	U32 dest_pid;
	U32 expiry;
	struct msgbuf *msg_env;
} MSG_HEADER;

int k_send_message(int process_id, void *message_envelope);
int k_send_message_nb(int process_id, void *message_envelope);
int k_delayed_send(int process_id, void *message_envelope, int delay);
void *k_receive_message(int *sender_id);
void *nb_receive_message(int *sender_id);

/* IPC Management */
extern int k_send_message(int pid, void *p_msg);
#define send_message(pid, p_msg) _send_message((U32)k_send_message, pid, p_msg)
extern int _send_message(U32 p_func, int pid, void *p_msg) __SVC_0;

extern void *k_receive_message(int *p_pid);
#define receive_message(p_pid) _receive_message((U32)k_receive_message, p_pid)
extern void *_receive_message(U32 p_func, void *p_pid) __SVC_0;

/* Timing Service */
extern int k_delayed_send(int pid, void *p_msg, int delay);
#define delayed_send(pid, p_msg, delay) _delayed_send((U32)k_delayed_send, pid, p_msg, delay)
extern int _delayed_send(U32 p_func, int pid, void *p_msg, int delay) __SVC_0;  
#endif /* ! MESSAGE_H_ */
