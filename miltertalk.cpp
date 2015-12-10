#include "milter_packet.h"
#include <unistd.h>
#include <netinet/in.h>
extern void dbg( const char * str);


#define SMFIF_ADDHDRS		(1L<<0)	// filter may add headers 
#define SMFIF_CHGBODY		(1L<<1)	// filter may replace body 
#define SMFIF_ADDRCPT		(1L<<2)	// filter may add recipients 
#define SMFIF_DELRCPT		(1L<<3)	// filter may delete recipients 
#define SMFIF_CHGHDRS		(1L<<4)	// filter may change/delete headers 
#define SMFIF_QUARANTINE 	(1L<<5)	// filter may quarantine envelope 
#define SMFIF_CHGFROM		(1L<<6)	// filter may replace sender 
#define SMFIF_ADDRCPT_PAR	(1L<<7)	// filter may add recipients + args 
#define SMFIF_SETSYMLIST	(1L<<8)	// filter may send macro names 
                                     	
#define SMFIP_NOCONNECT		(1L<<0)		// no connect info 
#define SMFIP_NOHELO		(1L<<1)		// no HELO info 
#define SMFIP_NOMAIL		(1L<<2)		// no MAIL info 
#define SMFIP_NORCPT		(1L<<3)		// no RCPT info 
#define SMFIP_NOBODY		(1L<<4)		/ no body 
#define SMFIP_NOHDRS		(1L<<5)		// no headers 
#define SMFIP_NOEOH		(1L<<6)		// no EOH 
#define SMFIP_NR_HDR		(1L<<7)		// no reply to header 
#define SMFIP_NOUNKNOWN 	(1L<<8)		// no unknown cmd 
#define SMFIP_NODATA		(1L<<9)		// no DATA 
#define SMFIP_SKIP		(1L<<10)	// MTA supports SMFIR_SKIP 
#define SMFIP_RCPT_REJ		(1L<<11)	// filter wants rejected RCPTs 
#define SMFIP_NR_CONN		(1L<<12)	// filter won't reply for connect 
#define SMFIP_NR_HELO		(1L<<13)	// filter won't reply for HELO 
#define SMFIP_NR_MAIL		(1L<<14)	// filter won't reply for MAIL 
#define SMFIP_NR_RCPT		(1L<<15)	// filter won't reply for RCPT 
#define SMFIP_NR_DATA		(1L<<16)	// filter won't reply for DATA 
#define SMFIP_NR_UNKN		(1L<<17)	// filter won't reply for UNKNOWN 
#define SMFIP_NR_EOH		(1L<<18)	// filter won't reply for eoh 
#define SMFIP_NR_BODY		(1L<<19)	// filter won't reply for body chunk 
#define SMFIP_HDR_LEADSPC	(1L<<20)	// header value has leading space 

//---------------------------------------------------------------------------
bool st_exit(int fd);
bool st_negotiation(int fd);
bool st_loop1(int fd);

bool (*st_ptr)(int);

//---------------------------------------------------------------------------
bool st_exit(int fd) {
	dbg("::exit");
	close(fd);
	return false;
}

bool st_negotiation(int fd) {
	dbg("::negotiation");
	milter_packet mp_ric;
	int ret=mp_ric.get(fd);
	if (ret==false) {
		dbg("Connection closed");
		st_ptr=st_exit;
		return false;
		}
	if (mp_ric.get_cmd()!='O') { //SMFIC_OPTNEG
		dbg("Expecting OPTION");
		st_ptr=st_exit;
		return false;
		}
	// sent option response
	milter_packet_option mpo(SMFIF_ADDHDRS,SMFIP_NOUNKNOWN); //SMFIF, SMFIP
	ret=mpo.put(fd);
	if (ret==false) {
		dbg("Connection closed sending options");
		st_ptr=st_exit;
		return false;
		}
	st_ptr=st_loop1;
	return true;
}	

bool st_loop1(int fd) {
	dbg("::loop1");
	milter_packet mp_ric;
	
	milter_packet_resp mp_continue('c');
	milter_packet_resp mp_reject('r');
	milter_packet_resp mp_tempfail('t');
	
	int ret=mp_ric.get(fd);
	if (ret==false) {
		dbg("Connection closed");
		st_ptr=st_exit;
		return false;
		}
	switch(mp_ric.get_cmd()) {
		case 'D': { // MACRO
			dbg("Macro definition");
			milter_packet_macro mpo(mp_ric);
			mpo.rawDump();
			return true; 
			}
		case 'C': { // SMFIC_CONNECT
			dbg("Connection");
			milter_packet_connect mpC(mp_ric);
			mpC.rawDump();
			mp_continue.put(fd);
			return true;
			}
		case 'H': { // SMFIC_HELO
			dbg("Helo");
			milter_packet_helo mph(mp_ric);
			mph.rawDump();
			mph.parse();
			mph.dump();
			mp_continue.put(fd);
			return true;
			}
		case 'M': { // SMFIC_MAIL
			dbg("Mail from");
			milter_packet_mailfrom mph(mp_ric);
			mph.rawDump();
			mph.parse();
			mph.dump();
			mp_continue.put(fd);
			return true;
			}
			
		case 'R': { // SMFIC_RCPT
			dbg("Rcpt to");
			milter_packet_rcptto mpr(mp_ric);
			mpr.rawDump();
			mpr.parse();
			mpr.dump();
			mp_continue.put(fd);
			return true;
			}
		case 'L': { // SMFIC_HEADER
			dbg("Header");
			milter_packet_header mph(mp_ric);
			mph.rawDump();
			mph.parse();
			mph.dump();
			mp_continue.put(fd);
			return true;
			}
		case 'N': { // SMFIC_EOH
			dbg("End of header");
			mp_ric.dump();
			mp_continue.put(fd);
			return true;
			}
		case 'B': { // SMFIC_BODY
			dbg("Body");
			milter_packet_body mpb(mp_ric);
			mpb.rawDump();
			mpb.parse();
			mpb.dump();
			mp_continue.put(fd);
			return true;
			}
		case 'E': { // SMFIC_BODYEOB
			dbg("End of body");
			mp_ric.dump();
			mp_continue.put(fd);
			return true;
			}
		case 'A': // SMFIC_ABORT
		case 'T': // SMFIC_DATA
		case 'Q': // SMFIC_QUIT
		default:
			dbg("-- TODO --");
			mp_ric.dump();
			mp_continue.put(fd);
			return true;
			
		}
}

void milter_talk(int fd) {
	st_ptr=st_negotiation;
	for (;;) {
		// wait for OPTNEG
		if ((*st_ptr)(fd)==false)
			break;
		}
	dbg("talkQ");
	close(fd);
}