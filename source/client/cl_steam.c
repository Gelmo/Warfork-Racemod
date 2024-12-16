#include "../qcommon/qcommon.h"
#include "../qcommon/steam.h"
#include "client.h"
#include "../steamshim/src/parent/parent.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

static void printEvent( const STEAMSHIM_Event *e )
{
	if( !steam_debug->integer || !e )
		return;

	Com_Printf( "%sokay, ival=%d, fval=%f, lval=%llu, name='%s').\n", e->okay ? "" : "!", e->ivalue, e->fvalue, e->lvalue, e->name );
} 

static const STEAMSHIM_Event* blockOnEvent(STEAMSHIM_EventType type){

	while( 1 ) {
		const STEAMSHIM_Event *evt = STEAMSHIM_pump();
		if (!evt) continue;

		if (evt->type == type){
			printEvent( evt );
			return evt;
		} else {
			printf("warning: ignoring event %i\n",evt->type);
			// event gets ignored!
			printEvent( evt );
		}
	}
}

/*
* Steam_RunFrame
*/
void CL_Steam_RunFrame( void )
{
	const STEAMSHIM_Event *evt = STEAMSHIM_pump();
	if( evt ) {
		printEvent( evt );
		switch (evt->type){
			case SHIMEVENT_AVATARRECIEVED: 
				{
          CL_GameModule_CallbackRequestAvatar(evt->lvalue, evt->name);
				}
				break;
			case SHIMEVENT_GAMEJOINREQUESTED:
				{
					uint64_t inviter = evt->lvalue;
					CL_ParseSteamConnectString(evt->name);
				}
				break;
			default: break;
		}
	}
}

/*
* Steam_GetAuthSessionTicketBlocking
*/
const SteamAuthTicket_t* Steam_GetAuthSessionTicketBlocking(){
	static SteamAuthTicket_t ticket;

	STEAMSHIM_getAuthSessionTicket();
	const STEAMSHIM_Event *evt = blockOnEvent(SHIMEVENT_AUTHSESSIONTICKETRECIEVED);

	ticket.pcbTicket = evt->lvalue;
	memcpy(ticket.pTicket, evt->name, AUTH_TICKET_MAXSIZE);

	return &ticket;
}


void Steam_GetPersonaName( char *name, size_t namesize )
{
	if( !namesize ) {
		return;
	}
	STEAMSHIM_getPersonaName();
	const STEAMSHIM_Event *evt = blockOnEvent(SHIMEVENT_PERSONANAMERECIEVED);
	strncpy(name, evt->name, namesize);
}

void Steam_OpenProfile(uint64_t steamid) { 
	STEAMSHIM_openProfile(steamid);
}

void Steam_SetRichPresence( int num, const char **key, const char **val )
{
	STEAMSHIM_setRichPresence(num, key, val);
}

uint64_t Steam_GetSteamID( void )
{
	STEAMSHIM_getSteamID();
	const STEAMSHIM_Event *evt = blockOnEvent(SHIMEVENT_STEAMIDRECIEVED);
	return evt->lvalue;
}

/*
* Steam_RequestAvatar
* size is 0 for 32x32, 1 for 64x64, 2 for 128x128
*/
void Steam_RequestAvatar(uint64_t steamid, int size)
{
	STEAMSHIM_requestAvatar(steamid, size);
}

/*
* Steam_AdvertiseGame
*/
void Steam_AdvertiseGame( const uint8_t *ip, unsigned short port )
{
	const char* keys[1] = {"connect"};
	const char* values[1];

	char connectstr[64];
	if (port) {
		snprintf( connectstr,sizeof connectstr, "%d.%d.%d.%d:%i",ip[0],ip[1],ip[2],ip[3],port);
		values[0] = connectstr;
	} else {
		values[0] = "";
	}
	Steam_SetRichPresence(1, keys, values);
}
