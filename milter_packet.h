#ifndef __MILTER_PACKET_H_
#define __MILTER_PACKET_H_

#include <cstdint>

class milter_packet {
	
	public:
		milter_packet();
		~milter_packet();
		bool get(int fd);
		bool put(int fd);
	private:
		uint32_t	m_len;
		char 		m_cmd;
		char		*m_data;
	
};

#endif //__MILTER_PACKET_H_