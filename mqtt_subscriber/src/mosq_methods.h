void setup_tls(struct mosquitto *mosq, struct security_configs security_conf);
void setup_user_authentication(struct mosquitto *mosq, struct security_configs security_conf);
void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg);
void on_connect(struct mosquitto *mosq, void *obj, int rc);
