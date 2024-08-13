/*
 * HypView - (c)      - 2006 Philipp Donze
 *               2006 -      Philipp Donze & Odd Skancke
 *
 * A replacement hypertext viewer
 *
 * This file is part of HypView.
 *
 * HypView is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * HypView is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HypView; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <string.h>
#ifdef __GNUC__
	#include <osbind.h>
#else
	#include <tos.h>
#endif
#include "include/av.h"
#include <gem.h>
#include "diallib.h"

#if USE_AV_PROTOCOL != NO


#if USE_AV_PROTOCOL >= 2			/*	normales/maximales AV-Protokoll	*/
void DoVA_PROTOSTATUS(short msg[8]);
void DoAV_PROTOKOLL(short flags);
void DoAV_EXIT(void);
#endif


#if USE_AV_PROTOCOL >= 2
/*
 *		Normales AV-Protokoll: Anmelden und Abmelden beim Server
 */

short server_id = 0;								/*	Programm ID des Servers	*/
long server_cfg = 0;
char *av_name=NULL;							/*	Eigener Programmname	*/


/************************************************
 *		VA-Befehle, bzw. Antworten vom Server		*
 ************************************************/

/*	Antwort des Servers auf AV_PROTOKOLL	*/
void DoVA_PROTOSTATUS(short msg[8])
{
	union {
		short msg[2];
		long l;
	} cfg;

	if(av_name)				/*	Applikationsname aus AV_PROTOKOLL noch da?	*/
	{
		Mfree(av_name);	/*	Speicher freigeben	*/
		av_name = NULL;
	}
	
	cfg.msg[0] = msg[3];
	cfg.msg[1] = msg[4];
	server_cfg = cfg.l;
#if DEBUG == ON
	Debug("Server name: %s  protocol: %lx",*(char **)&msg[6],server_cfg);
#endif
}

/************************************************
 *		AV-Befehle, bzw. Kommandos an den Server	*
 ************************************************/

/*	Anmeldung beim Server (unter Angabe des Protokolls)	*/
void DoAV_PROTOKOLL(short flags)
{
	char *avserver = NULL;
	
	if(shel_envrn(&avserver,"AVSERVER=") && avserver)
	{
		short msg[8] = {AV_PROTOKOLL,0,0,0,0,0};
		char avserver_name[9], *dst = avserver_name;
#if DEBUG==ON
		Debug("AVSERVER=%s",avserver);
#endif

		while (*avserver && (dst < &avserver_name[8]))
			*dst++ = *avserver++;

		while(dst < &avserver_name[8])
			*dst++ = ' ';
		*dst = 0;

		server_id = appl_find(avserver_name);
		if(server_id >= 0)
		{
			char **pname;
			
			if(!av_name)
			{
				av_name = (char *)Mxalloc(16, MX_PREFTTRAM|MX_MPROT|MX_GLOBAL);
				if ((long)av_name == -32l)
					av_name = (char *)Malloc(16);
				if(!av_name)
					return;
				
				strcpy(av_name, PROGRAM_UNAME);
			}
			else
			{
				/* av_name already allocated, server_cfg may be
				 * set or not set (in case we're still waiting
				 * for the reply VA_PROTOSTATUS).
				 * Don't send another AV_PROTOKOLL */
				return;
			}

#if DEBUG==ON
			Debug("ID: %d",server_id);
#endif
			msg[1] = ap_id;
			msg[3] = flags;
			pname = (char **)&msg[6];
			*pname = av_name;
			appl_write(server_id, 16, msg);
		}
#if DEBUG==ON
		else
			Debug("AVSERVER nicht gestartet!");
#endif
	}
#if DEBUG==ON
	else
		Debug("Kein AVSERVER gefunden!");
#endif
}

/*	Teilt dem Server mit, dass die Applikation nicht mehr am Protokoll teilnimmt	*/
void DoAV_EXIT(void)
{
	short msg[8] = {AV_EXIT,0,0,0,0,0,0,0};
		
	msg[1] = ap_id;
	msg[3] = ap_id;
	appl_write(server_id, 16, msg);
}

#endif	/* USE_AV_PROTOCOL >= 2	*/
#endif	/*	USE_AV_PROTOCOL != NO	*/
