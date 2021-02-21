#ifndef _TELNETADMIN_H_
#define _TELNETADMIN_H_

#include "config.h"

#define ADMIN_USAGE                    "$SYS $MEM $NET $WIF $UPD $RST ALL|ESP|OTGW $EXT $VER $HLP"

#ifdef USE_TELNET_ADMIN

extern void telnet_admin_server_setup(void);
extern void telnet_admin_server_loop(void);

#endif  // USE_TELNET_ADMIN


#endif  // _TELNETADMIN_H_