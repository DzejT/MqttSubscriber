#include <uci.h>
#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include "uci_utils.h"
#include "event_utils.h"
#include "mosq_methods.h"

int run_loop = 1;

void sigHandler(int signo);

int main(int argc, char **argv){
    openlog ("MQTTsub", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    syslog (LOG_INFO, "MQTT subscriber started");
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);


    int rc;
    mosquitto_lib_init();
    struct mosquitto *mosq;

    struct general_configs general_conf;
    struct security_configs security_conf;
    struct events events;

    read_configs(&general_conf, &security_conf, &events);
    
    struct callback_data data = {0};
    data.events = events;
    data.general_conf = general_conf;
    data.security_conf = security_conf;
    char *host = strdup(general_conf.host);

    mosq = mosquitto_new("MQTT subscriber", true, &data);

    mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_message_callback_set(mosq, on_message);


    if(security_conf.use_auth != 0)
        setup_user_authentication(mosq, security_conf);
    if(security_conf.use_tls != 0){
        setup_tls(mosq, security_conf);
    }

    rc = mosquitto_connect(mosq, host, general_conf.port, 10);
    free(host);

    if(rc){
        syslog(LOG_ERR, "mosquitto_connect failed | Error code : %d\n", rc);
    }

    mosquitto_loop_start(mosq);

    while(run_loop){

    }
    mosquitto_loop_stop(mosq, true);
    

    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    return 0;
}


void sigHandler(int signo) {
    signal(SIGINT, NULL);
    syslog (LOG_INFO,  "Received signal: %d\n", signo);
    run_loop = 0;
}


