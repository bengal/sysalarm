/*
 * config.h
 *
 */

#ifndef CONFIG_H_
#define CONFIG_H_

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
	int specific;
};

struct action {
	char *name;
	struct action_type *type;
	struct option_value *options;
	void *specific_config;
};

struct action_type {
	char *name;
	int (*set_options)(struct action *action, struct option_value *options);
	int (*trigger_action)(struct action *action);
};

struct condition {
	char *name;
	struct condition_type *type;
	struct option_value *options;
	struct action *action;
	void *specific_config;
};

struct condition_type {
	char *name;
	int (*set_options)(struct condition *condition, struct option_value *options);
	int (*check_condition)(struct condition *condition);
};

struct condition *new_condition();
struct action *new_action();
struct condition *search_condition(char *name);
struct action *search_action(char *name);
struct condition_type *search_condition_type(char *name);
struct action_type *search_action_type(char *name);

extern struct condition conditions[];
extern struct action actions[];

#endif /* CONFIG_H_ */
