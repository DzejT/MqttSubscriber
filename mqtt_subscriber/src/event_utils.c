#include <mosquitto.h>
#include <curl/curl.h>
#include "uci_utils.h"
#include <time.h>
#include <json-c/json.h>
#include <syslog.h>
#include "event_utils.h"

#define RC_OK 0
#define RC_ERR 1


struct upload_status {
    int line_read;
};

char payload_text[2048];

static size_t payload_source(char *ptr, size_t size, size_t nmemb, void *userp)
{
    struct upload_status *upload_ctx = (struct upload_status *)userp;
    const char *data;

    if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
        return 0;
    }
    
    if(upload_ctx->line_read == 0){
    
        data = payload_text;

        size_t len = strlen(data);
        memcpy(ptr, data, len);
        upload_ctx->line_read++;

        return len;
    }

    return 0;
}

static void send_mail(struct event_configs data, char *text)
{
  CURL *curl;
  CURLcode res = CURLE_OK;
  struct curl_slist *recipients = NULL;
  struct upload_status upload_ctx = { 0 };


  curl = curl_easy_init();
  if(curl) {
    snprintf(payload_text, 2048, "To: A recieved %s From: Router %s \r\nSubject: Event received\r\n\r\n%s\r\n", data.recipient, "jokubastrak@gmail.com", text);
    /* Set username and password */
    curl_easy_setopt(curl, CURLOPT_USERNAME, data.sender);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, data.password);
    curl_easy_setopt(curl, CURLOPT_URL, "smtp://smtp.gmail.com:587");

    curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, data.sender);

    recipients = curl_slist_append(recipients, data.recipient);
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

    curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
    curl_easy_setopt(curl, CURLOPT_READDATA, (void*)&upload_ctx);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

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
    int rc = parse_json(data.parameter, msg->payload, 1, &value);
    if(rc == 1)
    {
        return;
    }

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
    int rc = parse_json(data.parameter, msg->payload, 0, &value);
    if(rc != RC_OK){
        return;
    }
    

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

static int parse_json(char *param, char *msgPayload, int variableType, void *value){
    struct json_object *value_json, *data_json, *parsed_json;

    parsed_json = json_tokener_parse(msgPayload);
    json_bool rc = json_object_object_get_ex(parsed_json, "data", &data_json);
    if (!rc){
        syslog(LOG_ERR, "Failed to read JSON value");
        return RC_ERR;
    }

    rc = json_object_object_get_ex(data_json, param, &value_json);
    if (!rc){
        syslog(LOG_ERR, "Failed to read JSON value");
        return RC_ERR;
    }
        
    if(variableType == 0){
        *(char**)value = json_object_get_string(value_json);
        return RC_OK;
    }
    if(variableType == 1){
        *(int*)value = json_object_get_int(value_json);
        return RC_OK;
    }

}