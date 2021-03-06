/*
 * action_mail.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#include <auth-client.h>
#include <libesmtp.h>

#include "base.h"
#include "util.h"

#include "config.h"

#define SMTP_MONITOR 0

#define METHOD_LOCAL 1
#define METHOD_SMTP 2

#define MAIL_MSG_MAX_SIZE (128*1024)

struct mail_action_config {
	char *mail_from;
	char *mail_to;
	char *mail_subject;
	char *mail_smtp_server;
	int mail_smtp_port;
	char *mail_smtp_user;
	char *mail_smtp_pass;
	int mail_smtp_starttls;
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

#if SMTP_MONITOR
static void monitor_cb (const char *buf, int buflen, int writing, void *arg)
{
	FILE *fp = arg;

	if (writing == SMTP_CB_HEADERS) {
		fputs ("H: ", fp);
		fwrite (buf, 1, buflen, fp);
		return;
	}

	fputs (writing ? "C: " : "S: ", fp);
	fwrite (buf, 1, buflen, fp);
	if (buf[buflen - 1] != '\n')
		putc ('\n', fp);
}
#endif


static int auth_interact(auth_client_request_t request, char **result, int fields, void *arg)
{

	int i;
	struct mail_action_config *config = arg;

	for (i = 0; i < fields; i++) {
		if ((request[i].flags & AUTH_USER) && config->mail_smtp_user) {
			result[i] = config->mail_smtp_user;
		} else if ((request[i].flags & AUTH_PASS) && config->mail_smtp_pass) {
			result[i] = config->mail_smtp_pass;
		} else {
			printf("Credentials needed!\n");
			return 0;
		}
	}
	return 1;
}

static void ignore_sigpipe()
{
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigemptyset (&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGPIPE, &sa, NULL);
}


static void send_mail_smtp(struct mail_action_config *config, struct result *cond_res,
		struct result *result)
{
	smtp_session_t session;
	smtp_message_t message;
	auth_context_t authctx;
	char body[MAIL_MSG_MAX_SIZE];
	const smtp_status_t *status;
	char server[1024];

	ignore_sigpipe(); /* TODO: restore on return */

	auth_client_init();
	session = smtp_create_session();

#if SMTP_MONITOR
	smtp_set_monitorcb (session, monitor_cb, stdout, 1);
#endif

	authctx = auth_create_context ();
	auth_set_mechanism_flags (authctx, AUTH_PLUGIN_PLAIN, 0);
	auth_set_interact_cb(authctx, auth_interact, config);
	if(config->mail_smtp_starttls)
		smtp_starttls_enable(session, Starttls_ENABLED);
	smtp_auth_set_context(session, authctx);

	message = smtp_add_message(session);

	snprintf(server, sizeof(server), "%s:%d", config->mail_smtp_server, config->mail_smtp_port);
	smtp_set_server(session, server);

	smtp_set_header(message, "To", NULL, config->mail_to);
	smtp_set_header(message, "From", NULL, config->mail_from);
	smtp_set_header(message, "Subject", config->mail_subject);
	smtp_set_header(message, "X-Mailer", "sysalarm v" PACKAGE_VERSION " with libesmtp");
	smtp_add_recipient(message, config->mail_to);

	snprintf(body, MAIL_MSG_MAX_SIZE, "\r\nAn alarm condition was detected:\r\n%s",
			result_get_description(cond_res));
	smtp_set_message_str(message, body);

	if (!smtp_start_session(session)) {
		char buf[128];
		snprintf(result->desc, RESULT_DESC_LEN, "SMTP server problem: %s",
				smtp_strerror(smtp_errno(), buf, sizeof(buf)));
		result->code = ACTION_ERROR;
		return;
	} else {
		status = smtp_message_transfer_status(message);
		if(status->code == 250){
			result->code = ACTION_OK;
			return;
		} else {
			result->code = ACTION_ERROR;
			snprintf(result->desc, RESULT_DESC_LEN, "SMTP code %d - %s", status->code,
					(status->text == NULL ? "unknown" : status->text));
			return;
		}
	}

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

	if(config->mail_method == METHOD_SMTP && config->mail_smtp_server == NULL)
		die("Parameter 'smtp_server' is required for action MAIL, method smtp");
}

static int mail_action_set_options(struct action *action, struct option_value *options)
{
	struct option_value *option;
	struct mail_action_config *config = calloc(1, sizeof(struct mail_action_config));
	CHECK_MALLOC(config);
	action->priv_config = config;

	for (option = options; option != NULL; option = option->next) {

		if (!strcmp(option->name, "mail_from")) {
			config->mail_from = strdup(option->value);

		} else if (!strcmp(option->name, "mail_to")) {
			config->mail_to = strdup(option->value);

		} else if (!strcmp(option->name, "mail_subject")) {
			config->mail_subject = strdup(option->value);

		} else if (!strcmp(option->name, "mail_smtp_server")) {
			config->mail_smtp_server = strdup(option->value);

		} else if (!strcmp(option->name, "mail_smtp_port")) {
			config->mail_smtp_port = atoi(option->value);

		} else if (!strcmp(option->name, "mail_smtp_user")){
			config->mail_smtp_user = strdup(option->value);

		} else if (!strcmp(option->name, "mail_smtp_pass")){
			config->mail_smtp_pass = strdup(option->value);

		} else if (!strcmp(option->name, "mail_smtp_starttls")){
			config->mail_smtp_starttls = atoi(option->value);

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
	struct mail_action_config *config = action->priv_config;

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
