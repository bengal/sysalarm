/*
 * config.h
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#define SPECIFIC_CONFIG(dest,config) dest = (typeof (dest))config->specific_config

#define ALARM_ON 1
#define ALARM_OFF 2
#define ALARM_ERROR 3

#define BUF_LEN 2048
#define MAX_ALARM_NUM 1024

#define NOTIFY_METHOD_EMAIL 1
#define NOTIFY_METHOD_BEEP 2

#define CONF_EMAIL_METHOD_LOCAL 1
#define CONF_EMAIL_METHOD_SMTP 2

struct global_config {
	char *email_from;
	char *email_to;
	char *email_subject;
	int email_method;
};

struct alarm_condition {

	char *name;
	struct alarm_type *type;

	int notify_method;
	int check_interval;

	void *specific_config;
};

struct alarm_type {
	char *code;
	void (*init_alarm_config)(struct alarm_condition *);
	int (*parse_config_option)(struct alarm_condition *, char *, char *);
	void (*check_config)(struct alarm_condition *);
	int (*check_alarm)(struct alarm_condition *);
};


void parse_config_file(char *file);

extern struct alarm_condition alarms[];
extern struct global_config global_config;

#endif /* CONFIG_H_ */
