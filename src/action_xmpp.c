#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <iksemel.h>

#include "base.h"
#include "util.h"

/**
 *
 * Work in progress
 *
 */

struct xmpp_action_config {
	char *xmpp_server;
	int   xmpp_port;
	char *xmpp_jid;
	char *xmpp_passwd;
	char *xmpp_msg;
	char *xmpp_recip;
};

struct xmpp_session {
	iksparser *prs;
	iksid *acc;
	iksfilter *filter;
	char *passwd;
	int authorized;
	int features;
	char *recip;
	char *msg;
	int finished;
};

void xmpp_send_status(struct xmpp_session *session, char *status) {
	iks *pres = iks_make_pres(IKS_SHOW_AVAILABLE, status);
	iks_send(session->prs, pres);
	iks_delete(pres);
}

void xmpp_send_message(struct xmpp_session *session, char *to, char *message) {
	iks *msg;
	msg = iks_make_msg(IKS_TYPE_CHAT, to, message);
	iks_send(session->prs, msg);
	iks_delete(msg);
	session->finished = 1;
}

static int on_auth_result(struct xmpp_session *session, ikspak *pak) {
	xmpp_send_message(session, session->recip, "An alarm condition was detected:");
	xmpp_send_message(session, session->recip, session->msg);
	session->finished = 1;
	return IKS_FILTER_EAT;
}

static void on_log(struct xmpp_session *sess, const char *text, size_t size, int is_incoming)
{
	char *type = "RECV";
	char *secure = "";
	if (iks_is_secure (sess->prs)) secure = "Sec ";
	if (!is_incoming) type = "SEND";
	sa_log(SA_LOG_DEBUG, "%s%s [%s]\n", secure, type, text);
}

static int on_stream(struct xmpp_session *session, int type, iks *node) {
	switch(type) {
	case IKS_NODE_START:
		if (!iks_is_secure(session->prs))
			iks_start_tls(session->prs);
		break;
	case IKS_NODE_NORMAL:
		if (strcmp("stream:features", iks_name(node)) == 0) {
			session->features = iks_stream_features(node);
			if (!iks_is_secure(session->prs))
				break;
			if (session->authorized) {
				iks *t;
				if (session->features & IKS_STREAM_BIND) {
					t = iks_make_resource_bind(session->acc);
					iks_send(session->prs, t);
					iks_delete (t);
				}
				if (session->features & IKS_STREAM_SESSION) {
					t = iks_make_session();
					iks_insert_attrib(t, "id", "auth");
					iks_send(session->prs, t);
					iks_delete (t);
				}
			} else {
				if (session->features & IKS_STREAM_SASL_MD5)
					iks_start_sasl(session->prs, IKS_SASL_DIGEST_MD5, session->acc->user,
							session->passwd);
				else if (session->features & IKS_STREAM_SASL_PLAIN)
					iks_start_sasl(session->prs, IKS_SASL_PLAIN, session->acc->user,
							session->passwd);
			}
		}
		else if (strcmp("success", iks_name(node)) == 0) {
			session->authorized = 1;
			iks_send_header(session->prs, session->acc->server);
		}
		else if (strcmp("failure", iks_name(node)) == 0)
			sa_log(SA_LOG_DEBUG, "SASL authentication failed\n");
		else {
			ikspak *pak;
			pak = iks_packet(node);
			iks_filter_packet(session->filter, pak);
		}
		break;
	case IKS_NODE_STOP:
		sa_log(SA_LOG_DEBUG, "Server disconnected");
	case IKS_NODE_ERROR:
		sa_log(SA_LOG_DEBUG, "Stream error");
	default:
		sa_log(SA_LOG_DEBUG, "Unknown node type %d", type);
	}

	if (node)
		iks_delete(node);
	return IKS_OK;
}



struct xmpp_session *xmpp_connect(struct xmpp_action_config *config)
{
	struct xmpp_session *sess;
	int result;

	sess = calloc(1, sizeof(struct xmpp_session));
	CHECK_MALLOC(sess);

	sess->prs = iks_stream_new(IKS_NS_CLIENT, sess, (iksStreamHook *) on_stream);
	sess->acc =  iks_id_new(iks_parser_stack(sess->prs), config->xmpp_jid);
	sess->recip = config->xmpp_recip;
	sess->msg = "TODO";

	if (sess->acc->resource == NULL) {
		char *tmp = iks_malloc(strlen(sess->acc->user) + strlen(sess->acc->server)
				+ strlen("test") + 3);
		sprintf (tmp, "%s@%s/%s", sess->acc->user, sess->acc->server, "test");
		sess->acc = iks_id_new(iks_parser_stack(sess->prs), tmp);
		iks_free(tmp);
	}

	sess->passwd = config->xmpp_passwd;
	iks_set_log_hook(sess->prs, (iksLogHook *) on_log);
	sess->filter = iks_filter_new();

	iks_filter_add_rule(sess->filter, (iksFilterHook *) on_auth_result, sess,
			IKS_RULE_TYPE, IKS_PAK_IQ, IKS_RULE_SUBTYPE, IKS_TYPE_RESULT,
			IKS_RULE_ID, "auth", IKS_RULE_DONE);

	if (config->xmpp_server == NULL)
		config->xmpp_server = sess->acc->server;

	result = iks_connect_with(sess->prs, config->xmpp_server, config->xmpp_port,
			sess->acc->server, &iks_default_transport);

	switch(result) {
	case IKS_OK:
		sa_log(SA_LOG_DEBUG, "Connection OK\n");
		break;
	case IKS_NET_NODNS:
		sa_log(SA_LOG_DEBUG, "DNS lookup of '%s' failed", config->xmpp_server);
	case IKS_NET_NOCONN:
		sa_log(SA_LOG_DEBUG, "Connection to '%s' failed", config->xmpp_server);
	default:
		sa_log(SA_LOG_DEBUG, "Unspecified error %d from iksemel", result);
	}
	return sess;

}

static void xmpp_action_trigger_action(struct action *action, struct result *cond_res,
		struct result *result)
{
	struct xmpp_action_config *config = action->priv_config;
	struct xmpp_session *session = xmpp_connect(config);
	if(!session){
		set_result(result, ACTION_ERROR, "Could not connect to XMPP server");
		return;
	}

	/* TODO Work in Progress, does not work correctly */

	while(!session->finished){
		iks_recv (session->prs, 1);
	}
}


static void check_configuration(struct xmpp_action_config *config)
{
	if(config->xmpp_jid == NULL)
		die("Parameter 'xmpp_jid' is required for action XMPP");
	if(config->xmpp_passwd == NULL)
		die("Parameter 'xmpp_passwd' is required for action XMPP");
	if(config->xmpp_recip == NULL)
		die("Parameter 'xmpp_recip' is required for action XMPP");
}

static int xmpp_action_set_options(struct action *action, struct option_value *options)
{
	struct option_value *option;
	struct xmpp_action_config *config = calloc(1, sizeof(struct xmpp_action_config));
	CHECK_MALLOC(config);
	action->priv_config = config;
	config->xmpp_port = IKS_JABBER_PORT;

	for (option = options; option != NULL; option = option->next) {

		if (!strcmp(option->name, "xmpp_server")) {
			config->xmpp_server = strdup(option->value);

		} else if (!strcmp(option->name, "xmpp_port")) {
			config->xmpp_port = atoi(option->value);

		} else if (!strcmp(option->name, "xmpp_jid")) {
			config->xmpp_jid = strdup(option->value);

		} else if (!strcmp(option->name, "xmpp_passwd")) {
			config->xmpp_passwd = strdup(option->value);

		} else if (!strcmp(option->name, "xmpp_msg")) {
			config->xmpp_msg = strdup(option->value);

		} else if (!strcmp(option->name, "xmpp_recip")){
			config->xmpp_recip = strdup(option->value);

		} else {
			die("Unknown option '%s' for action '%s'", option->name, action->name);
		}
	}

	check_configuration(config);
	return 0;
}

struct action_type action_type_xmpp = {
		.name = "XMPP",
		.description = "Send a XMPP message",
		.set_options = xmpp_action_set_options,
		.trigger_action = xmpp_action_trigger_action,
};
