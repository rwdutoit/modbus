#include <signal.h> 
#include <stdio.h> 
#include <stdint.h> 
#include <string.h> 
#include <mosquitto.h> 
#define mqtt_host "m10.cloudmqtt.com" //"localhost"
#define user "yinigjid"
#define pass "X1tsmSciZwEI"
#define mqtt_port 11985 // 1883
#define myTopic  "counter"

static int run = 1; 


void handle_signal(int s) {
	run = 0;
}
void connect_callback(struct mosquitto *mosq, void *obj, 
int result) {
	printf("connect callback, rc=%d\n", result);
}
void message_callback(struct mosquitto *mosq, void *obj, 
const struct mosquitto_message *message) {
	bool match = 0;
	printf("got message '%.*s' for topic '%s'\n", 
message->payloadlen, (char*) message->payload, 
message->topic);
	mosquitto_topic_matches_sub("ADC", 
message->topic, &match);
	if (match) {
		printf("got message for ADC topic\n");
	}
}
int main(int argc, char *argv[]) {
	uint8_t reconnect = true;
	char clientid[24];
	struct mosquitto *mosq;
	int rc = 0; 
bool clean_session = true;
 int counter =0;
 int qos =2;
 char msg[10];
 bool retain = true;
	signal(SIGINT, handle_signal);
	signal(SIGTERM, handle_signal);
	mosquitto_lib_init();
	memset(clientid, 0, 24);
	snprintf(clientid, 23, "mysql_log_%d", getpid());
	mosq = mosquitto_new(clientid, true, 0);
	if(mosq)
	{
		mosquitto_connect_callback_set(mosq, connect_callback);
		mosquitto_message_callback_set(mosq, message_callback);
		mosquitto_username_pw_set(mosq,user,pass);
	    	rc = mosquitto_connect(mosq, mqtt_host,mqtt_port, 60);
		mosquitto_subscribe(mosq, NULL,"ADC", 0);
		while(run)
		{
			rc = mosquitto_loop(mosq, -1, 1);
			if(run && rc){
				printf("connection error!\n");
				sleep(10);
				mosquitto_reconnect(mosq);
			}

counter++;
//sprintf(msg, "%d", counter);
if((counter % 10) == 0)
{
  sprintf(msg, "%d", counter);
  int snd = mosquitto_publish(mosq, NULL, myTopic, strlen(msg), msg, qos, retain);
  if(snd != 0)
    printf("mqtt_send error=%i\n", snd);
}
		}
		mosquitto_destroy(mosq);
	}
	mosquitto_lib_cleanup();
	return rc;
}
