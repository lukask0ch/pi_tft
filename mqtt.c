#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"
#include "mqtt.h"

#define CLIENTID	"RaspberryPi1"
#define QOS		1
#define TIMEOUT		10000L

MQTTClient client;

volatile MQTTClient_deliveryToken deliveredtoken;

void delivered(void *context, MQTTClient_deliveryToken dt)
{
	//printf("Message with token value %d delivery confirmed\n", dt);
	deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
	int i;
	char* payloadptr;

	//printf("Message arrived\n");
	//printf("	topic: %s\n", topicName);
	//printf("	message: ");

	payloadptr = message->payload;
	for(i=0; i<message->payloadlen; i++)
	{
		putchar(*payloadptr++);
	}
	putchar('\n');
	MQTTClient_freeMessage(&message);
	MQTTClient_free(topicName);
	return 1;
}

void connlost(void *context, char *cause)
{
	//printf("\nConnection lost\n");
	//printf("	cause: %s\n", cause);

	connectToBroker();
}

void instantiateClient(char* address){
	int rc;
	do{
		//printf("Trying to instantiate\n");
		rc = MQTTClient_create(&client, address, CLIENTID,
				MQTTCLIENT_PERSISTENCE_NONE, NULL);
	} while(rc != MQTTCLIENT_SUCCESS);
	//printf("Instantiated MQTTClient\n");
	MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);
}

void connectToBroker(void){
	int rc;
	MQTTClient_connectOptions conn_opts =
			MQTTClient_connectOptions_initializer;
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 0;
	do{
		//printf("Trying to connect\n");
		rc = MQTTClient_connect(client, &conn_opts);
	} while(rc != MQTTCLIENT_SUCCESS);
	//printf("Connected to Broker\n");
}

void sendMessage(char* topic, char* payload){
	MQTTClient_message pubmsg = MQTTClient_message_initializer;
	MQTTClient_deliveryToken token;

	pubmsg.payload = payload;
	pubmsg.payloadlen = strlen(payload);
	pubmsg.qos = QOS;
	pubmsg.retained = 0;
	deliveredtoken = 0;
	MQTTClient_publishMessage(client, topic, &pubmsg, &token);
	//printf("Current token: %d\n", token);
	while(deliveredtoken != token);
	printf("Delivered %s\n", payload);
}

void disconnectFromBroker(void){
	MQTTClient_disconnect(client, TIMEOUT);
	MQTTClient_destroy(&client);
	//printf("Disconnected from Broker and destroyed MQTTClient\n");
}
