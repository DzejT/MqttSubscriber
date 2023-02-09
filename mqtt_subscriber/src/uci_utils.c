#include <uci.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uci_utils.h"



void read_configs(struct general_configs *general_conf, struct security_configs *security_conf, struct events *events)
{
    
    load_typed_sections(general_conf, events);
    load_other_configs(general_conf, security_conf);
}



void load_typed_sections(struct general_configs *configs, struct events *events){

    const char *config_name = "mqtt_sub";
    configs->n = 0;
    events->n = 0;

    struct uci_context *context = uci_alloc_context();
    struct uci_package *package;

    if (uci_load(context, config_name, &package) != UCI_OK)
    {
        uci_perror(context, "uci_load()");
        uci_free_context(context);
        return;
    }


    struct uci_element *i;
    uci_foreach_element(&package->sections, i)
    {
        struct uci_section *section = uci_to_section(i);
        char *section_name = section->type;

        if(strcmp(section_name, "interface") == 0){
            load_topics(configs, section);
            configs->n++;
        }

        if(strcmp(section_name, "event") == 0){
            load_events(events, section);
            events->n++;
        }     
    }
}


void load_topics(struct general_configs *configs, struct uci_section *section){

    struct uci_element *j;
    uci_foreach_element(&section->options, j)
    {
        struct uci_option *option = uci_to_option(j);
        if(strcmp(option->e.name, "topic") == 0){
            strncpy(configs->topic[configs->n], option->v.string, 49);
        }
    }

}

void load_events(struct events *events, struct uci_section *section){
    struct uci_element *j;
    uci_foreach_element(&section->options, j)
    {
        struct uci_option *option = uci_to_option(j);
        if(strcmp(option->e.name, "topic") == 0){
            events->event[events->n].topic = option->v.string;
        }

        if(strcmp(option->e.name, "comparator") == 0){
            events->event[events->n].comparator = option->v.string;
        }

        if(strcmp(option->e.name, "value") == 0){
            events->event[events->n].value = option->v.string;
        }

        if(strcmp(option->e.name, "type") == 0){
            events->event[events->n].type = option->v.string;
        }

        if(strcmp(option->e.name, "sender") == 0){
            events->event[events->n].sender = option->v.string;
        }

        if(strcmp(option->e.name, "reciever") == 0){
            events->event[events->n].recipient = option->v.string;
        }

        if(strcmp(option->e.name, "parameter") == 0){
            events->event[events->n].parameter = option->v.string;
        }

        if(strcmp(option->e.name, "password") == 0){
            events->event[events->n].password = option->v.string;
        }
    }
}


void load_other_configs(struct general_configs *general_conf, struct security_configs *security_conf){
    int rc = 0;
    struct uci_context *cursor;
    struct uci_ptr ptr;
    cursor = uci_alloc_context();
    // last parameter indicates whether variable is char* or int. 0 - char*, 1 - int
    load_config(cursor, ptr, "mqtt_sub.general.host", &general_conf->host, 0);
    load_config(cursor, ptr, "mqtt_sub.general.port", &general_conf->port, 1);
    load_config(cursor, ptr, "mqtt_sub.user_info.use_auth", &security_conf->use_auth, 1);
    load_config(cursor, ptr, "mqtt_sub.security.use_tls", &security_conf->use_tls, 1);
    if(security_conf->use_auth != 0){
        load_config(cursor, ptr, "mqtt_sub.user_info.username", &security_conf->username, 0);
        load_config(cursor, ptr, "mqtt_sub.user_info.password", &security_conf->password, 0);
    }
    if(security_conf->use_tls != 0){
        load_config(cursor, ptr, "mqtt_sub.security.tls_type", &security_conf->tls_type, 0);
        if(strcmp(security_conf->tls_type, "cert") == 0){
            load_config(cursor, ptr, "mqtt_sub.security.ca_file", &security_conf->ca_file_path, 0);
            load_config(cursor, ptr, "mqtt_sub.security.cert_file", &security_conf->cert_file_path, 0);
            load_config(cursor, ptr, "mqtt_sub.security.key_file", &security_conf->key_file_path, 0);
        }
        if(strcmp(security_conf->tls_type, "psk") == 0){
            load_config(cursor, ptr, "mqtt_sub.security.psk", &security_conf->psk, 0);
            load_config(cursor, ptr, "mqtt_sub.security.identity", &security_conf->identity, 0);
        }
    }


    uci_free_context(cursor);
}

void load_config(struct uci_context *cursor, struct uci_ptr ptr, char *option, void *variable, int variableType){
    int rc;
    char *config_path = strdup(option);
    if(config_path == NULL){
        printf("failed to malloc \n");
        return;
    }
    
    if(variableType == 0){
        char **var = variable;
        rc = uci_lookup_ptr (cursor, &ptr, config_path, true);
        if(rc != 0){
            printf("Unable to read config: %s", option);
            *var = NULL;
            free(config_path);
            return;
        }
        *var = ptr.o->v.string;    
    }

    else{
        int *var = variable;
        rc = uci_lookup_ptr (cursor, &ptr, config_path, true);
        if(rc != 0 || ptr.o->v.string == NULL){
            printf("Unable to read config: %s | RC: %d", option, rc);
            *var = 0;
            free(config_path);
            return;
        }
        *var = atoi(ptr.o->v.string);

    }

    free(config_path);
}
