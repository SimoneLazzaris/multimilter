#include "milter_packet.h"
#include <unistd.h>
extern void dbg( const char * str);

void milter_talk(int fd) {
	bool ret;
	milter_packet mp_ric,mp_tra;
	for (;;) {
		ret=mp_ric.get(fd);
		if (ret==false)
			break;
		ret=mp_tra.put(fd);
		if (ret==false)
			break;
	}
	close(fd);
}