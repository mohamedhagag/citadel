/*
 * $Id$ 
 *
 * Handle <iq> <get> <query> type situations (namespace queries)
 *
 * Copyright (c) 2007-2009 by Art Cancro
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "sysdep.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <pwd.h>
#include <errno.h>
#include <sys/types.h>

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include <sys/wait.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <expat.h>
#include <libcitadel.h>
#include "citadel.h"
#include "server.h"
#include "citserver.h"
#include "support.h"
#include "config.h"
#include "internet_addressing.h"
#include "md5.h"
#include "ctdl_module.h"
#include "serv_xmpp.h"


/*
 * Output a single roster item, for roster queries or pushes
 */
void xmpp_roster_item(struct CitContext *cptr) {
	char xmlbuf1[256];
	char xmlbuf2[256];

	cprintf("<item jid=\"%s\" name=\"%s\" subscription=\"both\">",
		xmlesc(xmlbuf1, cptr->cs_inet_email, sizeof xmlbuf1),
		xmlesc(xmlbuf2, cptr->user.fullname, sizeof xmlbuf2)
	);
	cprintf("<group>%s</group>", xmlesc(xmlbuf1, config.c_humannode, sizeof xmlbuf1));
	cprintf("</item>");
}

/* 
 * Return the results for a "jabber:iq:roster:query"
 *
 * Since we are not yet managing a roster, we simply return the entire wholist
 * (minus any entries for this user -- don't tell me about myself)
 *
 */
void xmpp_iq_roster_query(void)
{
	struct CitContext *cptr;
	int nContexts, i;

	cprintf("<query xmlns=\"jabber:iq:roster\">");
	cptr = CtdlGetContextArray(&nContexts);
	if (cptr) {
		for (i=0; i<nContexts; i++) {
			if (xmpp_is_visible(&cptr[i], CC)) {
				CtdlLogPrintf(CTDL_DEBUG, "Rosterizing %s\n", cptr[i].user.fullname);
				xmpp_roster_item(&cptr[i]);
			}
		}
		free (cptr);
	}
	cprintf("</query>");
}


/*
 * TODO: handle queries on some or all of these namespaces
 *
xmpp_query_namespace(purple5b5c1e58, splorph.xand.com, http://jabber.org/protocol/disco#items:query)
xmpp_query_namespace(purple5b5c1e59, splorph.xand.com, http://jabber.org/protocol/disco#info:query)
xmpp_query_namespace(purple5b5c1e5a, , vcard-temp:query)
 *
 */

void xmpp_query_namespace(char *iq_id, char *iq_from, char *iq_to, char *query_xmlns)
{
	int supported_namespace = 0;
	int roster_query = 0;
	char xmlbuf[256];

	/* We need to know before we begin the response whether this is a supported namespace, so
	 * unfortunately all supported namespaces need to be defined here *and* down below where
	 * they are handled.
	 */
	if (
		(!strcasecmp(query_xmlns, "jabber:iq:roster:query"))
		|| (!strcasecmp(query_xmlns, "jabber:iq:auth:query"))
	) {
		supported_namespace = 1;
	}

	CtdlLogPrintf(CTDL_DEBUG, "xmpp_query_namespace(%s, %s, %s, %s)\n", iq_id, iq_from, iq_to, query_xmlns);

	/*
	 * Beginning of query result.
	 */
	if (supported_namespace) {
		cprintf("<iq type=\"result\" ");
	}
	else {
		cprintf("<iq type=\"error\" ");
	}
	if (!IsEmptyStr(iq_from)) {
		cprintf("to=\"%s\" ", xmlesc(xmlbuf, iq_from, sizeof xmlbuf));
	}
	cprintf("id=\"%s\">", xmlesc(xmlbuf, iq_id, sizeof xmlbuf));

	/*
	 * Is this a query we know how to handle?
	 */

	if (!strcasecmp(query_xmlns, "jabber:iq:roster:query")) {
		roster_query = 1;
		xmpp_iq_roster_query();
	}

	else if (!strcasecmp(query_xmlns, "jabber:iq:auth:query")) {
		cprintf("<query xmlns=\"jabber:iq:auth\">"
			"<username/><password/><resource/>"
			"</query>"
		);
	}

	/*
	 * If we didn't hit any known query namespaces then we should deliver a
	 * "service unavailable" error (see RFC3921 section 2.4 and 11.1.5.4)
	 */

	else {
		CtdlLogPrintf(CTDL_DEBUG,
			"Unknown query namespace '%s' - returning <service-unavailable/>\n",
			query_xmlns
		);
		cprintf("<error code=\"503\" type=\"cancel\">"
			"<service-unavailable xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/>"
			"</error>"
		);
	}

	cprintf("</iq>");

	/* If we told the client who is on the roster, we also need to tell the client
	 * who is *not* on the roster.  (It's down here because we can't do it in the same
	 * stanza; this will be an unsolicited push.)
	 */
	if (roster_query) {
		xmpp_delete_old_buddies_who_no_longer_exist_from_the_client_roster();
	}
}