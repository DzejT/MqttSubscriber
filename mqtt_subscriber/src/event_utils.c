// #include "event_utils.h"
#include <mosquitto.h>
#include <curl/curl.h>
#include "uci_utils.h"
#include <time.h>
#include <json-c/json.h>
#include <syslog.h>


static size_t payload_source(char *ptr, size_t size, size_t nmemb, void *userp);
static void send_mail(struct event_configs data, char text[]);
void check_value(const struct mosquitto_message *msg, void *obj);
static void check_int_value(const struct mosquitto_message *msg, struct event_configs data);
static void check_string_value(const struct mosquitto_message *msg, struct event_configs data);
static void parse_json(char *param, char *msgPayload, int variableType, void *value);

struct upload_status {
    size_t bytes_read;
    char text[100];
    struct event_configs data
};

char payload_text[500] =
  "Date: Mon, 29 Nov 2023 21:54:29 +1100\r\n"
  "Subject: Router event notification\r\n"
  "\r\n" 
  "Tekstas, kuris įtikins Jokūbą, kad tikrai veikia :D.\r\n";



// char* generate_text(char *body, struct event_configs data){
//     char payload[500];
//     sprintf(payload, 
//     "Date: %s\r\n"
//     "To:  %s \r\n"
//     "From: %s \r\n"
//     "Subject: Event notification\r\n"
//     "\r\n" /* empty line to divide headers from body, see RFC5322 */
//     "%s.\r\n"
//     "\r\n", body, data.recipient, data.sender);

//     return payload;
// }
  
static size_t payload_source(char *ptr, size_t size, size_t nmemb, void *userp)
{
    struct upload_status *upload_ctx = (struct upload_status *)userp;
    const char *data;
    size_t room = size * nmemb;

    if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
        return 0;
    }

    // char *payload_text = generate_text(upload_ctx->text, upload_ctx->data);

    // char *payload;

    // strcat(payload_text, upload_ctx->text);

    data = &payload_text[upload_ctx->bytes_read];

    if(data) {
        size_t len = strlen(data);
        if(room < len)
        len = room;
        memcpy(ptr, data, len);
        upload_ctx->bytes_read += len;

        return len;
    }

    return 0;
}

static void send_mail(struct event_configs data, char text[])
{
  CURL *curl;
  CURLcode res = CURLE_OK;
  struct curl_slist *recipients = NULL;
  struct upload_status upload_ctx = { 0 };
  strncpy(upload_ctx.text, text, 100);
  upload_ctx.data = data;

  curl = curl_easy_init();
  if(curl) {
    /* Set username and password */
    curl_easy_setopt(curl, CURLOPT_USERNAME, "jokubastrak@gmail.com");
    curl_easy_setopt(curl, CURLOPT_PASSWORD, "lrknzmcinkkuyrgh");
    curl_easy_setopt(curl, CURLOPT_URL, "smtp://smtp.gmail.com:587");

    curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, data.sender);

    recipients = curl_slist_append(recipients, data.recipient);
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

    curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
    curl_easy_setopt(curl, CURLOPT_READDATA, (void*)&upload_ctx);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    res = curl_easy_perform(curl);

    if(res != CURLE_OK)
      syslog(LOG_ERR, "\ncurl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    curl_slist_free_all(recipients);

    curl_easy_cleanup(curl);
  }

  return (int)res;
}

void check_value(const struct mosquitto_message *msg, void *obj)
{
    struct callback_data *data = obj;


    for(int i = 0; i < data->events.n; i++){
        if(strcmp(data->events.event[i].type, "num") == 0){
            check_int_value(msg, data->events.event[i]);
        }
        else if(strcmp(data->events.event[i].type, "alph") == 0){
            check_string_value(msg, data->events.event[i]);
        }
    }
    
}

static void check_int_value(const struct mosquitto_message *msg, struct event_configs data){
    data.int_value = atoi(data.value);
    char text [100];
    int value;
    parse_json(data.parameter, msg->payload, 1, &value);
    printf("value\n");
    printf("%d\n", value);

    // if(value == NULL)
    //     return;

    // struct json_object *value_json, *data_json, *parsed_json;

    // parsed_json = json_tokener_parse(msg->payload);
    // bool rc = json_object_object_get_ex(parsed_json, "data", &data_json);
    // if (rc == false)
    //     return;
    
    // rc = json_object_object_get_ex(data_json, data.parameter, &value_json);
    // if (rc == false)
    //     return;

    // int value = json_object_get_int(value_json);
    

    if(strcmp(data.comparator, ">") == 0){
        if(value > data.int_value){
            sprintf(text, "Notification:\n %s: %d > %d", data.parameter, value, data.int_value);
            syslog("send email to %s | %d %s %d \n", data.recipient, data.int_value, data.comparator, value);
            send_mail(data, text);
        }
    }
    else if(strcmp(data.comparator, ">=") == 0){
        if(value >= data.int_value){
            sprintf(text, "Notification:\n %s: %d >= %d", data.parameter, value, data.int_value);
            syslog("send email to %s | %d %s %d \n", data.recipient, data.int_value, data.comparator, value);
            send_mail(data, text);
        }
    }
    else if(strcmp(data.comparator, "<") == 0){
        if(value < data.int_value){
            sprintf(text, "Notification:\n %s: %d < %d", data.parameter, value, data.int_value);
            send_mail(data, text);
            syslog("send email to %s | %d %s %d \n", data.recipient, data.int_value, data.comparator, value);
        }
    }
    else if(strcmp(data.comparator, "<=") == 0){
        if(value <= data.int_value){
            sprintf(text, "Notification:\n %s: %d <= %d", data.parameter, value, data.int_value);
            syslog("send email to %s | %d %s %d \n", data.recipient, data.int_value, data.comparator, value);
            send_mail(data, text);
        }
    }
    else if(strcmp(data.comparator, "==") == 0){
        if(value == data.int_value){
            sprintf(text, "Notification:\n %s: %d == %d", data.parameter, value, data.int_value);
            syslog("send email to %s | %d %s %d \n", data.recipient, data.int_value, data.comparator, value);
            send_mail(data, text);
        }
    }
    else if(strcmp(data.comparator, "!=") == 0){
        if(value != data.int_value){
            sprintf(text, "Notification:\n %s: %d != %d", data.parameter, value, data.int_value);
            syslog("send email to %s | %d %s %d \n", data.recipient, data.int_value, data.comparator, value);
            send_mail(data, text);
        }
    }
}

static void check_string_value(const struct mosquitto_message *msg, struct event_configs data){
    char text [100];
    char *value; 
    parse_json(data.parameter, msg->payload, 0, &value);
    if(value == NULL)
        return;
    
    printf("value\n");
    printf("%s\n", value);
    // struct json_object *value_json, *data_json, *parsed_json;

    // parsed_json = json_tokener_parse(msg->payload);
    // int rc = json_object_object_get_ex(parsed_json, "data", &data_json);
    // if (rc == NULL)
    //     return NULL;
    
    // rc = json_object_object_get_ex(data_json, data.parameter, &value_json);
    // if (rc == NULL)
    //     return NULL;

    // char *value = json_object_get_string(value_json);

    if(strcmp(data.comparator, ">") == 0){
        if(strcmp(value, data.value) > 0){
            sprintf(text, "Notification:\n %s: %s > %s", data.parameter, value, value);
            syslog("send email to %s | %s %s %s \n", data.recipient, data.value, data.comparator, value);
            send_mail(data, text);
        }
    }
    else if(strcmp(data.comparator, ">=") == 0){
        if(strcmp(value, data.value) >= 0){
            sprintf(text, "Notification:\n %s: %s >= %s", data.parameter, value, value);
            syslog("send email to %s | %s %s %s \n", data.recipient, data.value, data.comparator, value);
            send_mail(data, text);
        }
    }
    else if(strcmp(data.comparator, "<") == 0){
        if(strcmp(value, data.value) < 0){
            sprintf(text, "Notification:\n %s: %s < %s", data.parameter, value, value);
            syslog("send email to %s | %s %s %s \n", data.recipient, data.value, data.comparator, value);
            send_mail(data, text);
        }
    }
    else if(strcmp(data.comparator, "<=") == 0){
        if(strcmp(value, data.value) <= 0){
            sprintf(text, "Notification:\n %s: %s <= %s", data.parameter, value, data.value);
            syslog("send email to %s | %s %s %s \n", data.recipient, data.value, data.comparator, value);
            send_mail(data, text);
        }
    }
    else if(strcmp(data.comparator, "==") == 0){
        if(strcmp(value, data.value) == 0){
            sprintf(text, "Notification:\n %s: %s == %s", data.parameter, value, data.value);
            syslog("send email to %s | %s %s %s \n", data.recipient, data.value, data.comparator, value);
            send_mail(data, text);
        }
    }
    else if(strcmp(data.comparator, "!=") == 0){
        if(strcmp(value, data.value) != 0){
            sprintf(text, "Notification:\n %s: %s != %s", data.parameter, value, data.value);
            syslog("send email to %s | %s %s %s \n", data.recipient, data.value, data.comparator, value);
            send_mail(data, text);
        }
    }
}

static void parse_json(char *param, char *msgPayload, int variableType, void *value){
    char **charValue;
    int *intValue;
    printf("0\n");
    struct json_object *value_json, *data_json, *parsed_json;
    printf("1\n");

    parsed_json = json_tokener_parse(msgPayload);
    printf("2\n");
    int rc = json_object_object_get_ex(parsed_json, "data", &data_json);
    printf("3\n");
    if (rc == NULL)
        return NULL;
    
    printf("4\n");
    rc = json_object_object_get_ex(data_json, param, &value_json);
    if (rc == NULL)
        return NULL;
    

    printf("5\n");
    if(variableType == 0){
        *(char**)value = json_object_get_string(value_json);
        return;
    }
    printf("6\n");
    if(variableType == 1){
        *(int*)value = json_object_get_int(value_json);
        return;
    }

}