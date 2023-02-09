static size_t payload_source(char *ptr, size_t size, size_t nmemb, void *userp);
static void send_mail(struct event_configs data, char *text);
void check_value(const struct mosquitto_message *msg, void *obj);
static void check_int_value(const struct mosquitto_message *msg, struct event_configs data);
static void check_string_value(const struct mosquitto_message *msg, struct event_configs data);
static int parse_json(char *param, char *msgPayload, int variableType, void *value);