/*
 * action_mail.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <libesmtp.h>
#include <auth-client.h>

#include "base.h"
#include "util.h"

#define METHOD_LOCAL 1
#define METHOD_SMTP 2

#define MAIL_MSG_MAX_SIZE (128*1024)

struct mail_action_config {
	char *mail_from;
	char *mail_to;
	char *mail_subject;
	char *smtp_server;
	int mail_method;
};

static void prepare_mail_message(struct mail_action_config *config,
		char *buffer, int size, struct result *cond_res)
{
	time_t rawtime;
	char timestr[256];
	struct tm *timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(timestr, 256, "%c", timeinfo);

	snprintf(buffer, size,
			"From: %s\nTo: %s\nSubject: %s\nDate: %s\n\n"
			"An alarm condition was detected:\n%s",
			config->mail_to, config->mail_from, config->mail_subject,
			timestr, result_get_description(cond_res));
}

static void send_mail_smtp(struct mail_action_config *config, struct result *cond_res,
		struct result *result)
{

	smtp_session_t session;
	smtp_message_t message;
	smtp_recipient_t recipient;
	auth_context_t authctx;
	char body[MAIL_MSG_MAX_SIZE];
	const smtp_status_t *status;
	char server[1024];

	/* WiP */

	//prepare_mail_message(config, buffer, MAIL_MSG_MAX_SIZE, cond_res);
	snprintf(body, MAIL_MSG_MAX_SIZE, "An alarm condition was detected:\n%s",
			result_get_description(cond_res));

	auth_client_init ();
	session = smtp_create_session ();
	message = smtp_add_message (session);

	smtp_set_header (message, "To", NULL, config->mail_to);
	snprintf(server, 1024, "%s:%d", config->smtp_server, 25);
	smtp_set_server (session, server);
	smtp_add_recipient(message, config->mail_to);
	smtp_set_message_str(message, body);

	if (!smtp_start_session(session)) {
		char buf[128];
		//fprintf(stderr, "SMTP server problem %s\n", smtp_strerror(smtp_errno(), buf, sizeof buf));
		snprintf(result->desc, RESULT_DESC_LEN, "SMTP server problem: %s",
				smtp_strerror(smtp_errno(), buf, sizeof(buf)));

	} else {
		/* Report on the success or otherwise of the mail transfer.
		 */
		status = smtp_message_transfer_status(message);
		printf("%d %s", status->code, (status->text != NULL) ? status->text : "\n");
		//smtp_enumerate_recipients(message, print_recipient_status, NULL);
	}

	/* Free resources consumed by the program.
	 */
	smtp_destroy_session(session);
	auth_destroy_context(authctx);


	result->code =  ACTION_OK;
}

static void send_mail_local(struct mail_action_config *config, struct result *cond_res,
		struct result *result)
{
	char buffer[MAIL_MSG_MAX_SIZE];
	FILE *pipe;

	if((pipe = popen("/usr/lib/sendmail -t", "w")) == NULL){
		result->code = ACTION_ERROR;
		snprintf(result->desc, RESULT_DESC_LEN,
				"Error calling /usr/lib/sendmail. Verify it is installed"
				" and configured properly.");
		return;
	}

	prepare_mail_message(config, buffer, MAIL_MSG_MAX_SIZE, cond_res);
	fprintf(pipe, "%s", buffer);
	pclose(pipe);

	result->code = ACTION_OK;
	return;
}




static void check_configuration(struct mail_action_config *config)
{
	if(config->mail_from == NULL)
		die("Parameter 'mail_from' is required for action MAIL");
	if(config->mail_to == NULL)
		die("Parameter 'mail_to' is required for action MAIL");
	if(config->mail_subject == NULL)
		die("Parameter 'mail_subject' is required for action MAIL");

	if(config->mail_method == METHOD_SMTP && config->smtp_server == NULL)
		die("Parameter 'smtp_server' is required for action MAIL, method smtp");
}


static int mail_action_set_options(struct action *action, struct option_value *options)
{
	struct option_value *option;
	struct mail_action_config *config = calloc(1, sizeof(struct mail_action_config));
	CHECK_MALLOC(config);
	action->specific_config = config;

	for (option = options; option != NULL; option = option->next) {

		if(!option->specific)
			continue;

		if (!strcmp(option->name, "mail_from")) {
			config->mail_from = strdup(option->value);

		} else if (!strcmp(option->name, "mail_to")) {
			config->mail_to = strdup(option->value);

		} else if (!strcmp(option->name, "mail_subject")) {
			config->mail_subject = strdup(option->value);

		} else if (!strcmp(option->name, "smtp_server")) {
			config->smtp_server = strdup(option->value);

		} else if (!strcmp(option->name, "mail_method")) {

			if(!strcmp(option->value, "local")){
				config->mail_method = METHOD_LOCAL;

			} else if(!strcmp(option->value, "smtp")){
				config->mail_method = METHOD_SMTP;

			} else {
				die("Unknown mail method %s", option->value);
			}
		} else {
			die("Unknown option '%s' for action '%s'", option->name, action->name);
		}
	}

	check_configuration(config);

	return 0;
}


static void mail_action_trigger_action(struct action *action, struct result *cond_res,
		struct result *result)
{
	struct mail_action_config *config = action->specific_config;

	if(config->mail_method == METHOD_LOCAL){
		send_mail_local(config, cond_res, result);
	} else {
		send_mail_smtp(config, cond_res, result);
	}
}


struct action_type action_type_mail = {
	.name = "MAIL",
	.description = "Sends an email",
	.set_options = mail_action_set_options,
	.trigger_action = mail_action_trigger_action,
};
