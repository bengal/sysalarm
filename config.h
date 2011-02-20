/*
 * config.h
 *
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#define MAX_ELEMENTS 100

struct option {
	char *name;
	char *value;
	struct pair *next;
};

struct action {
	char *name;
	struct action_type *type;
	struct option *options;
};

struct action_type {
	char *name;
	int (*set_options)(struct action *action, struct option *options);
	int (*trigger_action)(struct action *action);
};

struct condition {
	char *name;
	struct condition_type *type;
	struct option *options;
};

struct condition_type {
	char *name;
	struct action *action;
	int (*set_options)(struct condition *condition, struct option *options);
	int (*check_condition)(struct condition *condition);
};


#endif /* CONFIG_H_ */
