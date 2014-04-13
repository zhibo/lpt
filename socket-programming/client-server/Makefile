CC=gcc
CLIENT=client.c
SERVER=server.c
TGT_CLIENT=client
TGT_SERVER=server


all: client server

client:
	$(CC) -o ${TGT_CLIENT} ${CLIENT}
server:
	$(CC) -o ${TGT_SERVER} ${SERVER}

clean:
	rm ${TGT_CLIENT} ${TGT_SERVER}
