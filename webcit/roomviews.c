/*
 * Lots of different room-related operations.
 */

#include "webcit.h"
#include "webserver.h"

char *viewdefs[VIEW_MAX];			/* the different kinds of available views */

ROOM_VIEWS exchangeable_views[VIEW_MAX][VIEW_MAX] = {	/* the different kinds of available views for a view */
{VIEW_BBS, VIEW_MAILBOX, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX }, 
{VIEW_BBS, VIEW_MAILBOX, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX }, 
{VIEW_MAX, VIEW_MAX, VIEW_ADDRESSBOOK, VIEW_CALENDAR, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX }, 
{VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_CALENDAR, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX /*VIEW_CALBRIEF*/, VIEW_MAX, VIEW_MAX }, 
{VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_TASKS, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX, },
{VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_NOTES, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX, },
{VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_WIKI, VIEW_MAX, VIEW_MAX, VIEW_MAX}, 
{VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_CALENDAR, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX/*VIEW_CALBRIEF*/, VIEW_MAX, VIEW_MAX},
{VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_JOURNAL, VIEW_MAX }, 
{VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_MAX, VIEW_BLOG }, 
	};
/* the brief calendar view is disabled: VIEW_CALBRIEF */

ROOM_VIEWS allowed_default_views[VIEW_MAX] = {
	1, /* VIEW_BBS		Bulletin board view */
	1, /* VIEW_MAILBOX		Mailbox summary */
	1, /* VIEW_ADDRESSBOOK	Address book view */
	1, /* VIEW_CALENDAR		Calendar view */
	1, /* VIEW_TASKS		Tasks view */
	1, /* VIEW_NOTES		Notes view */
	1, /* VIEW_WIKI		Wiki view */
	0, /* VIEW_CALBRIEF		Brief Calendar view */
	0, /* VIEW_JOURNAL		Journal view */
	0  /* VIEW_BLOG		Blog view (not yet implemented) */
};


/*
 * Initialize the viewdefs with localized strings
 */
void initialize_viewdefs(void) {
	viewdefs[VIEW_BBS] = _("Bulletin Board");
	viewdefs[VIEW_MAILBOX] = _("Mail Folder");
	viewdefs[VIEW_ADDRESSBOOK] = _("Address Book");
	viewdefs[VIEW_CALENDAR] = _("Calendar");
	viewdefs[VIEW_TASKS] = _("Task List");
	viewdefs[VIEW_NOTES] = _("Notes List");
	viewdefs[VIEW_WIKI] = _("Wiki");
	viewdefs[VIEW_CALBRIEF] = _("Calendar List");
	viewdefs[VIEW_JOURNAL] = _("Journal");
	viewdefs[VIEW_BLOG] = _("Blog");
}



void tmplput_ROOM_COLLECTIONTYPE(StrBuf *Target, WCTemplputParams *TP) 
{
	folder *Folder = (folder *)CTX;
	
	switch(Folder->view) {
	case VIEW_CALENDAR:
		StrBufAppendBufPlain(Target, HKEY("vevent"), 0);
		break;
	case VIEW_TASKS:
		StrBufAppendBufPlain(Target, HKEY("vtodo"), 0);
		break;
	case VIEW_ADDRESSBOOK:
		StrBufAppendBufPlain(Target, HKEY("vcard"), 0);
		break;
	case VIEW_NOTES:
		StrBufAppendBufPlain(Target, HKEY("vnotes"), 0);
		break;
	case VIEW_JOURNAL:
		StrBufAppendBufPlain(Target, HKEY("vjournal"), 0);
		break;
	case VIEW_WIKI:
		StrBufAppendBufPlain(Target, HKEY("wiki"), 0);
		break;
	}
}



int ConditionalRoomHasGroupdavContent(StrBuf *Target, WCTemplputParams *TP)
{
	folder *Folder = (folder *)CTX;

	lprintf(0, "-> %s: %ld\n", ChrPtr(Folder->name), Folder->view);

	return ((Folder->view == VIEW_CALENDAR) || 
		(Folder->view == VIEW_TASKS) || 
		(Folder->view == VIEW_ADDRESSBOOK) ||
		(Folder->view == VIEW_NOTES) ||
		(Folder->view == VIEW_JOURNAL) );
}




int ConditionalIsRoomtype(StrBuf *Target, WCTemplputParams *TP)
{
	wcsession *WCC = WC;

	if ((WCC == NULL) ||
	    (TP->Tokens->nParameters < 3))
	{
		return ((WCC->CurRoom.view < VIEW_BBS) || 
			(WCC->CurRoom.view > VIEW_MAX));
	}

	return WCC->CurRoom.view == GetTemplateTokenNumber(Target, TP, 2, VIEW_BBS);
}


void tmplput_CurrentRoomViewString(StrBuf *Target, WCTemplputParams *TP) 
{
	wcsession *WCC = WC;
	StrBuf *Buf;

	if ((WCC == NULL) ||
	    (WCC->CurRoom.defview >= VIEW_MAX) || 
	    (WCC->CurRoom.defview < VIEW_BBS))
	{
		LogTemplateError(Target, "Token", ERR_PARM2, TP,
				 "Roomview [%ld] not valid\n", 
				 (WCC != NULL)? 
				 WCC->CurRoom.defview : -1);
		return;
	}

	Buf = NewStrBufPlain(_(viewdefs[WCC->CurRoom.defview]), -1);
	StrBufAppendTemplate(Target, TP, Buf, 0);
	FreeStrBuf(&Buf);
}

void tmplput_RoomViewString(StrBuf *Target, WCTemplputParams *TP) 
{
	long CheckThis;
	StrBuf *Buf;

	CheckThis = GetTemplateTokenNumber(Target, TP, 0, 0);
	if ((CheckThis >= VIEW_MAX) || (CheckThis < VIEW_BBS))
	{
		LogTemplateError(Target, "Token", ERR_PARM2, TP,
				 "Roomview [%ld] not valid\n", 
				 CheckThis);
		return;
	}

	Buf = NewStrBufPlain(_(viewdefs[CheckThis]), -1);
	StrBufAppendTemplate(Target, TP, Buf, 0);
	FreeStrBuf(&Buf);
}



int ConditionalIsAllowedDefaultView(StrBuf *Target, WCTemplputParams *TP)
{
	wcsession *WCC = WC;
	long CheckThis;
	
	if (WCC == NULL)
		return 0;

	CheckThis = GetTemplateTokenNumber(Target, TP, 2, 0);
	if ((CheckThis >= VIEW_MAX) || (CheckThis < VIEW_BBS))
	{
		LogTemplateError(Target, "Conditional", ERR_PARM2, TP,
				 "Roomview [%ld] not valid\n", 
				 CheckThis);
		return 0;
	}

	return allowed_default_views[CheckThis] != 0;
}

int ConditionalThisRoomDefView(StrBuf *Target, WCTemplputParams *TP)
{
	wcsession *WCC = WC;
	long CheckThis;

	if (WCC == NULL)
		return 0;

	CheckThis = GetTemplateTokenNumber(Target, TP, 2, 0);
	return CheckThis == WCC->CurRoom.defview;
}

int ConditionalThisRoomCurrView(StrBuf *Target, WCTemplputParams *TP)
{
	wcsession *WCC = WC;
	long CheckThis;

	if (WCC == NULL)
		return 0;

	CheckThis = GetTemplateTokenNumber(Target, TP, 2, 0);
	return CheckThis == WCC->CurRoom.view;
}

int ConditionalThisRoomHaveView(StrBuf *Target, WCTemplputParams *TP)
{
	wcsession *WCC = WC;
	long CheckThis;
	
	if (WCC == NULL)
		return 0;

	CheckThis = GetTemplateTokenNumber(Target, TP, 2, 0);
	if ((CheckThis >= VIEW_MAX) || (CheckThis < VIEW_BBS))
	{
		LogTemplateError(Target, "Conditional", ERR_PARM2, TP,
				 "Roomview [%ld] not valid\n", 
				 CheckThis);
		return 0;
	}

	return exchangeable_views [WCC->CurRoom.defview][CheckThis] != VIEW_MAX;
}

void tmplput_ROOM_VIEW(StrBuf *Target, WCTemplputParams *TP) 
{
	folder *Folder = (folder *)CTX;
	StrBufAppendPrintf(Target, "%d", Folder->view);
}
void tmplput_ROOM_DEFVIEW(StrBuf *Target, WCTemplputParams *TP) 
{
	folder *Folder = (folder *)CTX;
	StrBufAppendPrintf(Target, "%d", Folder->defview);
}


void tmplput_CurrentRoomDefView(StrBuf *Target, WCTemplputParams *TP) 
{
	wcsession *WCC = WC;

	StrBufAppendPrintf(Target, "%d", WCC->CurRoom.defview);
}

void 
InitModule_ROOMVIEWS
(void)
{
	initialize_viewdefs();

	RegisterNamespace("THISROOM:VIEW_STRING", 0, 1, tmplput_CurrentRoomViewString, NULL, CTX_NONE);
	RegisterNamespace("ROOM:VIEW_STRING", 1, 2, tmplput_RoomViewString, NULL, CTX_NONE);

	RegisterConditional(HKEY("COND:ALLOWED_DEFAULT_VIEW"), 0, ConditionalIsAllowedDefaultView, CTX_NONE);
	RegisterConditional(HKEY("COND:THISROOM:DEFAULT_VIEW"), 0, ConditionalThisRoomDefView, CTX_NONE);
	RegisterNamespace("THISROOM:DEFAULT_VIEW", 0, 0, tmplput_CurrentRoomDefView, NULL, CTX_NONE);
	RegisterNamespace("ROOM:INFO:DEFVIEW", 0, 1, tmplput_ROOM_DEFVIEW, NULL, CTX_ROOMS);

	RegisterConditional(HKEY("COND:ROOM:TYPE_IS"), 0, ConditionalIsRoomtype, CTX_NONE);

	RegisterConditional(HKEY("COND:THISROOM:HAVE_VIEW"), 0, ConditionalThisRoomHaveView, CTX_NONE);
	RegisterConditional(HKEY("COND:ROOM:GROUPDAV_CONTENT"), 0, ConditionalRoomHasGroupdavContent, CTX_ROOMS);

	RegisterConditional(HKEY("COND:THISROOM:CURR_VIEW"), 0, ConditionalThisRoomCurrView, CTX_NONE);
	RegisterNamespace("ROOM:INFO:VIEW", 0, 1, tmplput_ROOM_VIEW, NULL, CTX_ROOMS);

	RegisterNamespace("ROOM:INFO:COLLECTIONTYPE", 0, 1, tmplput_ROOM_COLLECTIONTYPE, NULL, CTX_ROOMS);



}