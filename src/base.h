/*
 * base.h
 *
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <time.h>

#define MAX_ELEMENTS 100
#define BUF_LEN 2048

#define CONDITION_ON 1
#define CONDITION_OFF 2
#define CONDITION_ERROR 3

#define ACTION_OK 4
#define ACTION_ERROR 5

struct option_value {
	char *name;
	char *value;
	struct option_value *next;
};

#define RESULT_DESC_LEN 1024

/* Result of a condition or action */
struct result {
	int code;                      /* result code */
	char desc[RESULT_DESC_LEN];    /* result description */
	char *ext_desc;		       /* optional extended description */
};

struct action {
	char *name;
	struct action_type *type;
	void *priv_config;
};

struct action_type {
	char *name;
	char *description;
	int (*set_options)(struct action *action, struct option_value *options);
	void (*trigger_action)(struct action *action, struct result *cond_result,
			struct result *result);
};

struct cond_reg_action {
	struct action *action;
	int stop;
};

struct condition {
	/* configuration fields */
	char *name;
	struct condition_type *type;
	struct option_value *options;
	struct cond_reg_action **actions;
	int hold_time;
	int inactive_time;
	void *priv_config;

	/* from state file */
	time_t first_true;
	time_t last_alarm;
};

struct condition_type {
	char *name;
	char *description;
	int (*set_options)(struct condition *condition, struct option_value *options);
	void (*check_condition)(struct condition *condition, struct result *result);
};



struct condition *new_condition();
struct action *new_action();
struct condition *search_condition(char *name);
struct action *search_action(char *name);
struct condition_type *search_condition_type(char *name);
struct action_type *search_action_type(char *name);

char *result_get_description(struct result *);

extern struct condition conditions[];
extern struct action actions[];

#endif /* CONFIG_H_ */
