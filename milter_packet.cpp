#include "milter_packet.h"
#include <iostream>
#include <unistd.h>

milter_packet::milter_packet() {
	m_len=0;
	m_cmd=0;
	m_data=NULL;
}

milter_packet::~milter_packet() {
	m_len=0;
	m_cmd=0;
	if (m_data!=NULL)
		delete [] m_data;
	m_data=NULL;
}

bool milter_packet::get(int fd) {
	if (read(fd,(void*)&m_len,sizeof(m_len))<=0)
		return false;
	if (read(fd,(void*)&m_cmd,sizeof(m_cmd))<=0)
		return false;
	if (m_len>65536) return false;
	if (m_len<=1) return true;

	std::cerr << "PACKET: IN  <= " << m_cmd << "["<<m_len<<"]"<<std::endl;

	int l=m_len-1;
	m_data=new char[l];
	for (int o=0, r=0; l>0; l=l-r,o=o+r) {
		r=read(fd,m_data+o,l);
		if (r<=0) return false;
	}
	return true;
}

bool milter_packet::put(int fd) {
	if (write(fd,(void*)&m_len,sizeof(m_len))<=0)
		return false;
	if (write(fd,(void*)&m_cmd,sizeof(m_cmd))<=0)
		return false;

	std::cerr << "PACKET: OUT => " << m_cmd << "["<<m_len<<"]"<<std::endl;

	int l=m_len-1;
	for (int o=0, r=0; l>0; l=l-r,o=o+r) {
		r=write(fd,m_data+o,l);
		if (r<=0) return false;
	}
	return true;
}
