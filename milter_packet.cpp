#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <netinet/in.h>
#include "milter_packet.h"

milter_packet::milter_packet() {
	m_len=0;
	m_cmd=0;
	m_data=NULL;
}

milter_packet::milter_packet(milter_packet &mp):milter_packet() {
	m_data=NULL;
	m_cmd=mp.m_cmd;
	if (mp.m_data!=NULL)
		set_data(mp.m_data,mp.m_len-1);
}

milter_packet::~milter_packet() {
	m_len=0;
	m_cmd=0;
	if (m_data!=NULL)
		delete [] m_data;
	m_data=NULL;
}

bool milter_packet::get(int fd) {
	uint32_t t;
	if (read(fd,(void*)&t,sizeof(t))<=0)
		return false;
	m_len=ntohl(t);
	if (read(fd,(void*)&m_cmd,sizeof(m_cmd))<=0)
		return false;
	std::cerr << "PACKET: IN  <= " << m_cmd << "["<<m_len<<"]"<<std::endl;
	
	if (m_len>65536) return false;
	if (m_len<=1) return true;

	int l=m_len-1;
	m_data=new char[l];
	for (int o=0, r=0; l>0; l=l-r,o=o+r) {
		r=read(fd,m_data+o,l);
		if (r<=0) return false;
	}
/* // debug
	std::cerr<<"   DATA: ";
	std::cerr.setf(std::ios::hex, std::ios::basefield);
	for (int i=0; i<m_len-1; i++) std::cerr <<std::hex<< (int)(m_data[i])<<"["<<(m_data[i])<<"]"<<" ";
	std::cerr.unsetf(std::ios::hex);
	std::cerr<<std::endl;
*/
	return true;
}

bool milter_packet::put(int fd) {
	uint32_t t;
	t=htonl(m_len);
	if (write(fd,(void*)&t,sizeof(t))<=0)
		return false;
	if (write(fd,(void*)&m_cmd,sizeof(m_cmd))<=0)
		return false;

/* // debug
	std::cerr << "PACKET: OUT => " << m_cmd << "["<<m_len<<"]"<<std::endl;
	std::cerr<<"   DATA: ";
	std::cerr.setf(std::ios::hex, std::ios::basefield);
	for (int i=0; i<m_len-1; i++) std::cerr <<std::hex<< (int)(m_data[i])<<"["<<(m_data[i])<<"]"<<" ";
	std::cerr.unsetf(std::ios::hex);
	std::cerr<<std::endl;
*/
	int l=m_len-1;
	for (int o=0, r=0; l>0; l=l-r,o=o+r) {
		r=write(fd,m_data+o,l);
		if (r<=0) return false;
	}
	return true;
}

void milter_packet::set_data(const char *data, size_t len) {
	if (m_data!=NULL) delete [] m_data;
	m_data=new char[len];
	memcpy(m_data,data,len);
	m_len=len+1;
}

void milter_packet::set_cmd(char c) {
	m_cmd=c;
}

char milter_packet::get_cmd(void) {
	return m_cmd;
}

void milter_packet::rawDump(void) {
	std::cerr << "GENERIC PACKET RAW DUMP: " << m_cmd << "["<<m_len<<"]"<<std::endl;
	std::cerr.setf(std::ios::hex, std::ios::basefield);
	for (int i=0; i<m_len-1; i++) std::cerr << std::setfill('0') << std::setw(2)<< std::hex<< (int)(m_data[i])<<" ";
	std::cerr<<std::endl;
	for (int i=0; i<m_len-1; i++) std::cerr <<(m_data[i]);
	std::cerr.unsetf(std::ios::hex);
	std::cerr<<std::endl;
	std::cerr.unsetf(std::ios::hex);
	std::cerr<<std::endl;
}

void milter_packet::dump(void) {
	std::cerr << "GENERIC PACKET DUMP: " <<std::endl;
	
}

void milter_packet::parse(void) {
}

std::string milter_packet::parse_popString(void) {
	std::string ret("");
	for (int l=m_len-1; m_data[m_parseOffset]!=0 && m_parseOffset<l; m_parseOffset++) {
		ret=ret+m_data[m_parseOffset];
	}
	m_parseOffset++;
	return ret;
}

uint32_t milter_packet::parse_popInt32(void) {
	uint32_t ret=0;
	if (m_parseOffset+sizeof(uint32_t)<m_len-1) {
		ret=*(reinterpret_cast<uint32_t*>(m_data+m_parseOffset));
		m_parseOffset+=sizeof(uint32_t);
		}
	return ret;
}

uint16_t milter_packet::parse_popInt16(void) {
	uint16_t ret=0;
	if (m_parseOffset+sizeof(uint16_t)<m_len-1) {
		ret=*(reinterpret_cast<uint16_t*>(m_data+m_parseOffset));
		m_parseOffset+=sizeof(uint16_t);
		}
	return ret;
}

char milter_packet::parse_popChar(void){
	char ret=0;
	if (m_parseOffset+sizeof(char)<m_len-1) {
		ret=m_data[m_parseOffset];
		m_parseOffset+=sizeof(char);
		}
	return ret;
}


//---------------------------------------------------------------------------


milter_packet_option::milter_packet_option(unsigned action_mask, unsigned protocol_mask) {
	int32_t version=htonl(6);
	int32_t actions=htonl(action_mask);  //SMIF
	int32_t protocol=htonl(protocol_mask); //SMIP
	
	m_data=new char[12];
	memcpy(m_data,&version,sizeof(version));
	memcpy(m_data+4,&actions,sizeof(actions));
	memcpy(m_data+8,&protocol,sizeof(protocol));
	m_cmd='O';
	m_len=13;
}

//---------------------------------------------------------------------------


void milter_packet_macro::rawDump(void) {
	std::cerr << "MACRO PACKET RAW DUMP: "<< m_cmd << "["<<m_len<<"]"<<std::endl;
	std::cerr << " CMDCODE: "<<m_data[0]<<std::endl;
	for (int j=1,i=1,xx=0; i<m_len-1; i++) {
		if (m_data[i]==0) {
			if (xx==0) std::cerr << " NAME: ";
			if (xx==1) std::cerr << " VALUE: ";
			std::cerr << m_data+j;
			j=i+1;
			if (++xx==2) {
				std::cerr << std::endl;
				xx=0;
				}
			}
		}
}
//---------------------------------------------------------------------------


void milter_packet_connect::rawDump(void) {
	std::cerr << "CONNECT PACKET RAW DUMP: "<< m_cmd << "["<<m_len<<"]"<<std::endl;
	int offset=0,i;
	for (i=offset; i<m_len-1; i++) {
		if (m_data[i]==0)
			break;
		}
	if (i==m_len-1) return;
	std::cerr << " HOSTNAME: " << m_data+offset << std::endl;
	offset=i+1;
	char proto=m_data[offset++];
	std::cerr << " PROTOCOL: " <<proto<<std::endl;
	if (proto=='U') return;
	uint16_t port;
	port=*(reinterpret_cast<int*>(m_data+offset));
	std::cerr << " PORT: "<<ntohs(port);
	offset+=2;
	for (int i=offset; i<m_len-1; i++) {
		if (m_data[i]==0)
			break;
		}
	if (i==m_len-1) return;
	std::cerr << " ADDRESS: " <<m_data+offset<<std::endl;

}
//---------------------------------------------------------------------------
void milter_packet_helo::rawDump(void) {
	std::cerr << "HELO PACKET RAW DUMP: "<< m_cmd << "["<<m_len<<"]"<<std::endl;
	int offset=0,i;
	for (i=offset; i<m_len-1; i++) {
		if (m_data[i]==0)
			break;
		}
	if (i==m_len-1) return;
	std::cerr << " HELO NAME: " << m_data+offset << std::endl;
}

void milter_packet_helo::parse(void) {
	m_parseOffset=0;
	m_heloName=parse_popString();
}

void milter_packet_helo::dump(void) {
	std::cerr << "HELO PACKET: heloname: "<<m_heloName<<std::endl;
}

//---------------------------------------------------------------------------
void milter_packet_mailfrom::rawDump(void) {
	std::cerr << "MAILFROM PACKET RAW DUMP: "<< m_cmd << "["<<m_len<<"]"<<std::endl;
	int offset=0,i;
	for (i=offset; i<m_len-1; i++) {
		if (m_data[i]==0)
			break;
		}
	if (i==m_len-1) return;
	std::cerr << " MAIL FROM: " << m_data+offset << std::endl;
	offset=i+1;
	while (offset<m_len-1) {
		for (i=offset; i<m_len-1; i++) {
			if (m_data[i]==0)
				break;
			}
		if (i==m_len-1) return;
		std::cerr << " ESMTP PARAM: " << m_data+offset << std::endl;
		offset=i+1;
		}
}

void milter_packet_mailfrom::parse(void) {
	m_parseOffset=0;
	m_sender=parse_popString();
}

void milter_packet_mailfrom::dump(void) {
	std::cerr << "MAILFROM PACKET: mail from: "<<m_sender<<std::endl;
}

//---------------------------------------------------------------------------

void milter_packet_rcptto::rawDump(void) {
	std::cerr << "RCPTTO PACKET RAW DUMP: "<< m_cmd << "["<<m_len<<"]"<<std::endl;
	int offset=0,i;
	for (i=offset; i<m_len-1; i++) {
		if (m_data[i]==0)
			break;
		}
	if (i==m_len-1) return;
	std::cerr << " RCPT TO: " << m_data+offset << std::endl;
	offset=i+1;
	while (offset<m_len-1) {
		for (i=offset; i<m_len-1; i++) {
			if (m_data[i]==0)
				break;
			}
		if (i==m_len-1) return;
		std::cerr << " ESMTP PARAM: " << m_data+offset << std::endl;
		offset=i+1;
		}
}

void milter_packet_rcptto::parse(void) {
	m_parseOffset=0;
	m_rcpt=parse_popString();
}

void milter_packet_rcptto::dump(void) {
	std::cerr << "RCPTTO PACKET: rcpt to: "<<m_rcpt<<std::endl;
}

//---------------------------------------------------------------------------

void milter_packet_header::rawDump(void) {
	std::cerr << "HEADER PACKET RAW DUMP: "<< m_cmd << "["<<m_len<<"]"<<std::endl;
	int offset=0,i;
	for (i=offset; i<m_len-1; i++) {
		if (m_data[i]==0)
			break;
		}
	if (i==m_len-1) return;
	std::cerr << " HEADER NAME: " << m_data+offset << std::endl;
	offset=i+1;
	for (i=offset; i<m_len-1; i++) {
		if (m_data[i]==0)
			break;
		}
	if (i==m_len-1) return;
	std::cerr << " HEADER VALUE: " << m_data+offset << std::endl;
	offset=i+1;
}

void milter_packet_header::parse(void) {
	m_parseOffset=0;
	m_headName=parse_popString();
	m_headValue=parse_popString();
}

void milter_packet_header::dump(void) {
	std::cerr << "HEADER PACKET: "<<m_headName<<" = "<< m_headValue<<std::endl;
}

//---------------------------------------------------------------------------

void milter_packet_body::rawDump(void) {
	std::cerr << "BODY PACKET RAW DUMP: "<< m_cmd << "["<<m_len<<"]"<<std::endl;
	int offset=0,i;
	for (i=offset; i<m_len-1; i++) {
		if (m_data[i]==0)
			break;
		}
	if (i==m_len-1) return;
	std::cerr << " BODY: " << m_data+offset << std::endl;
	offset=i+1;
}

void milter_packet_body::parse(void) {
	m_parseOffset=0;
	m_body=parse_popString();
}

void milter_packet_body::dump(void) {
	std::cerr << "BODY PACKET: "<<m_body<<std::endl;
}

