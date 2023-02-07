struct general_configs{
    char *host;
    int port;
    char topic[10][50];
    int n;
};

struct security_configs{
    int use_tls;
    int use_auth; //for username and password
    char *username;
    char *password;
    char *ca_file_path;
    char *cert_file_path;
    char *key_file_path;
    char *tls_type;
    char *psk;
    char *identity;
};

struct event_configs{
    char *topic;
    char *comparator;
    char *value;
    int int_value;
    char *type;
    char *sender;
    char *recipient;
    char *parameter;
};

struct events{
    struct event_configs event[100];
    int n;
};

struct callback_data{
    struct general_configs general_conf;
    struct security_configs security_conf;
    struct events events;
};


void read_configs(struct general_configs *general_conf, struct security_configs *security_conf, struct events *events);
void load_typed_sections(struct general_configs *configs, struct events *events);
void load_topics(struct general_configs *configs, struct uci_section *section);
void load_events(struct events *events, struct uci_section *section);
void load_other_configs(struct general_configs *general_conf, struct security_configs *security_conf);
void load_config(struct uci_context *cursor, struct uci_ptr ptr, char *option, void *variable, int variableType);