#ifndef MESSAGE_H_
#define MESSAGE_H_

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
int k_delayed_send(int process_id, void *message_envelope, int delay);
void *k_receive_message(int *sender_id);
void *nb_receive_message(int *sender_id);

#endif /* ! MESSAGE_H_ */
