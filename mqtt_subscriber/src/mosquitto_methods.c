#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include "uci_utils.h"
#include "event_utils.h"

#define LOG_FILE "/log/mqtt_sub.log"


void on_connect(struct mosquitto *mosq, void *obj, int rc){

    struct callback_data *conf = obj;

    if(rc){
        syslog(LOG_ERR,"Error with result code: %d\n", rc);
        return;
    }

    for(int i = 0; i < conf->general_conf.n; i++){
        rc = mosquitto_subscribe(mosq, NULL, conf->general_conf.topic[i], 0);
        if(rc != 0)
            return;
    }
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg){
    syslog(LOG_INFO, "New message with topic %s: %s\n", msg->topic, (char*) msg->payload);
    FILE *fp;
    fp = fopen(LOG_FILE, "a");
    fprintf(fp, "New message with topic %s: %s\n", msg->topic, (char*) msg->payload);
    fclose(fp);

    check_value(msg, obj);
}

void setup_user_authentication(struct mosquitto *mosq, struct security_configs security_conf){
    int rc = mosquitto_username_pw_set(mosq, "user", "user");
    if(rc)
        syslog(LOG_ERR, "Failed to set username/password | RC = %d", rc);
    
}

void setup_tls(struct mosquitto *mosq, struct security_configs security_conf){
    int rc;
    if(strcmp(security_conf.tls_type, "cert") == 0)
        rc = mosquitto_tls_set(mosq, security_conf.ca_file_path, NULL, security_conf.cert_file_path, security_conf.key_file_path, NULL);
    
    else if(strcmp(security_conf.tls_type, "psk") == 0)
        rc = mosquitto_tls_psk_set(mosq, security_conf.psk, security_conf.identity, NULL);
    
    if(rc)
        syslog(LOG_ERR, "Failed to setup tls | RC = %d", rc);
}   

