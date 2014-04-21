CC=gcc
CLIENT=relay_client.c
SERVER=relay_server.c
TGT_CLIENT=client
TGT_SERVER=server


all: client server

client:
	$(CC) -o ${TGT_CLIENT} ${CLIENT}
server:
	$(CC) -o ${TGT_SERVER} ${SERVER}

clean:
	rm ${TGT_CLIENT} ${TGT_SERVER}
