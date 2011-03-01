/*
 * action_mail.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "config.h"
#include "util.h"

#define METHOD_LOCAL 1
#define METHOD_SMTP 2

struct mail_action_config {
	char *mail_from;
	char *mail_to;
	char *mail_subject;
	char *smtp_server;
	int mail_method;
};

static void send_mail_smtp(struct mail_action_config *config, struct result *cond_res,
		struct result *result)
{
	int sockfd;
	FILE *sock;
	char hostname[256];
	time_t rawtime;
	char timestr[256];
	struct tm *timeinfo;
	char message[1024];

	if((sockfd = connect_tcp(config->smtp_server, 25)) == -1){
		result->code = ACTION_ERROR;
		snprintf(result->desc, RESULT_DESC_LEN, "Could not connecto to %s",
				config->smtp_server);
		return;
	}

	if((sock = fdopen(sockfd, "w")) == NULL){
		result->code = ACTION_ERROR;
		snprintf(result->desc, RESULT_DESC_LEN, "fdopen error");
		return;
	}

	if(gethostname(hostname, 256) == -1){
		strncpy(hostname, "sysalarm", 256);
	}

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(timestr, 256, "%c", timeinfo);

	snprintf(message, 1024, "An alarm condition was detected:\n%s",
			result_get_description(cond_res));

	fprintf(sock, "HELO %s\n", hostname);
	fflush(sock);
	fprintf(sock, "MAIL FROM: <%s>\r\n", config->mail_from);
	fflush(sock);
	fprintf(sock, "RCPT TO: <%s>\n", config->mail_to);
	fflush(sock);
	fprintf(sock, "DATA\n");
	fflush(sock);
	fprintf(sock, "From: %s\nTo: %s\nSubject: %s\nDate: %s\n%s\r\n.\r\n",
			config->mail_from, config->mail_to, config->mail_subject,
			timestr, message);
	fprintf(sock, "QUIT\n");
	fflush(sock);

	fclose(sock);

	result->code =  ACTION_OK;
}

static void send_mail_local(struct mail_action_config *config, struct result *cond_res,
		struct result *result)
{
	debug("Sending an email using local method\n");

	FILE *pipe;
	if((pipe = popen("/usr/lib/sendmail -t", "w")) == NULL){
		result->code = ACTION_ERROR;
		snprintf(result->desc, RESULT_DESC_LEN,
				"Error calling /usr/lib/sendmail. Verify it is installed"
				" and configured properly.");
		return;
	}
	fprintf(pipe, "To: %s\n", config->mail_to);
	fprintf(pipe, "From: %s\n", config->mail_from);
	fprintf(pipe, "Subject: %s\n\n", config->mail_subject);
	fprintf(pipe, "TODO insert body here\n\n");
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
	.set_options = mail_action_set_options,
	.trigger_action = mail_action_trigger_action,
};
