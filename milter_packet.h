#ifndef __MILTER_PACKET_H_
#define __MILTER_PACKET_H_

#include <cstdint>
#include <string.h>
#include <string>

class milter_packet {
	
	public:
		milter_packet();
		milter_packet(milter_packet &m);
		virtual ~milter_packet();
		bool get(int fd);
		bool put(int fd);
		void set_data(const char *data, size_t len);
		void set_cmd(char c);
		char get_cmd(void);
		virtual void dump(void);
		virtual void rawDump(void);
		virtual void parse(void);
		
	protected:
		uint32_t	m_len;
		char 		m_cmd;
		char		*m_data;
		int		m_parseOffset;
		
		std::string	parse_popString(void);
		uint32_t	parse_popInt32(void);
		uint16_t	parse_popInt16(void);
		char		parse_popChar(void);
	
	};

class milter_packet_option:public milter_packet {
	public:
		milter_packet_option(milter_packet &mp):milter_packet(mp){};
		milter_packet_option(unsigned action_mask, unsigned protocol_mask);
	};


class milter_packet_macro:public milter_packet {
	public:
		milter_packet_macro(milter_packet &mp):milter_packet(mp){};
		void rawDump(void);
	};

class milter_packet_connect:public milter_packet {
	public:
		milter_packet_connect(milter_packet &mp):milter_packet(mp){};
		void rawDump(void);
	};

class milter_packet_resp:public milter_packet {
	public:
		milter_packet_resp(char r):milter_packet(){ m_len=1; m_cmd=r;};
	};

class milter_packet_helo:public milter_packet {
	public:
		milter_packet_helo(milter_packet &mp):milter_packet(mp){};
		void rawDump(void);
		void dump(void);
		void parse(void);
	private:
		std::string m_heloName;
	};

class milter_packet_mailfrom:public milter_packet {
	public:
		milter_packet_mailfrom(milter_packet &mp):milter_packet(mp){};
		void rawDump(void);
		void dump(void);
		void parse(void);
	private:
		std::string m_sender;
	};

class milter_packet_rcptto:public milter_packet {
	public:
		milter_packet_rcptto(milter_packet &mp):milter_packet(mp){};
		void rawDump(void);
		void dump(void);
		void parse(void);
	private:
		std::string m_rcpt;
	};

class milter_packet_header:public milter_packet {
	public:
		milter_packet_header(milter_packet &mp):milter_packet(mp){};
		void rawDump(void);
		void dump(void);
		void parse(void);
	private:
		std::string m_headName;
		std::string m_headValue;
	};

class milter_packet_body:public milter_packet {
	public:
		milter_packet_body(milter_packet &mp):milter_packet(mp){};
		void rawDump(void);
		void dump(void);
		void parse(void);
	private:
		std::string m_body;
	};


#endif //__MILTER_PACKET_H_