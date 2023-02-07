// #include <curl/curl.h>



// int send_email();
void check_value(const struct mosquitto_message *msg, void *obj);
static void send_mail(struct event_configs data);
static void *parse_json(char *param, const char *msgPayload, int variableType);
static void check_int_value(const struct mosquitto_message *msg, struct event_configs data);
static void check_string_value(const struct mosquitto_message *msg, struct event_configs data);
static size_t payload_source(char *ptr, size_t size, size_t nmemb, void *userp);
// char* generate_text(char *body);