/**
  * total hack to fix the problems where libkn has to be installed in 
  * /usr/local/kn/libnkn  for libkn to find this file prng_cmds for 
  * pseudo random initilaization. while this is really ugly, it is meant
  * as a quick fix solution for 1.3, particularly given this bug was 
  * discovered 2 days before release.
  */

#ifndef _PRNG_CMDS_H_
#define _PRNG_CMDS_H_

const char *__prngcmds=
"# entropy gathering commands\n"
"\n"
"# Format is: \"program-name args\" path rate\n"
"\n"
"# The \"rate\" represents the number of bits of usuable entropy per \n"
"# byte of command output. Be conservative.\n"
"#\n"
"# $Id: prng_cmds.h,v 1.2 2003/03/19 05:36:47 ifindkarma Exp $\n"
"\n"
"\"ls -alni /var/log\"			ls	0.02\n" 
"#\"ls -alni /var/adm\"			ls	0.02\n"
"\"ls -alni /var/mail\"			ls	0.02\n"
"#\"ls -alni /var/adm/syslog\"		ls	0.02\n"
"#\"ls -alni /var/spool/mail\"		ls	0.02\n"
"#\"ls -alni /proc\"			ls	0.02\n"
"\"ls -alni /tmp\"				ls	0.02\n"
"\"ls -alni /var/tmp\"			ls	0.02\n"
"#\"ls -alni /usr/tmp\"			ls	0.02\n"
"\"ls -alTi /var/log\"			ls	0.02\n"
"#\"ls -alTi /var/adm\"			ls	0.02\n"
"\"ls -alTi /var/mail\"			ls	0.02\n"
"#\"ls -alTi /var/adm/syslog\"		ls	0.02\n"
"#\"ls -alTi /var/spool/mail\"		ls	0.02\n"
"#\"ls -alTi /proc\"			ls	0.02\n"
"\"ls -alTi /tmp\"				ls	0.02\n"
"\"ls -alTi /var/tmp\"			ls	0.02\n"
"#\"ls -alTi /usr/tmp\"			ls	0.02\n"
"\n"
"\"netstat -an\"				netstat	0.05\n"
"#\"netstat -in\"				netstat	0.05\n"
"#\"netstat -rn\"				netstat	0.02\n"
"#\"netstat -pn\"				netstat	0.02\n"
"\"netstat -s\"				netstat	0.02\n"
"\n"
"\"arp -a -n\"					0.02\n"
"\n"
"\"ifconfig -a\"					0.02\n"
"\n"
"\"ps laxww\"				ps	0.03\n"
"\"ps -al\"				ps	0.03\n"
"#\"ps -efl\"				ps	0.03\n"
"\n"
"\"w\"					@PROG_W@	0.05\n"
"\n"
"#\"who -i\"				who	0.01\n"
"\n"
"\"last\"					last	0.01\n"
"\n"
"#\"lastlog\"					0.01\n"
"\n"
"\"df -l\"					df	0.01\n"
"\"df -il\"				df	0.01\n"
"\n"
"#\"vmstat\"				vmstat	0.01\n"
"\"uptime\"				uptime	0.01\n"
"\n"
"#\"ipcs -a\"				ipcs	0.01\n"
"\n"
"#\"tail -200 /var/log/messages\"		tail	0.01\n"
"\"tail -200 /var/log/syslog\"		tail	0.01\n"
"#\"tail -200 /var/adm/messages\"		tail	0.01\n"
"#\"tail -200 /var/adm/syslog\"		tail	0.01\n"
"#\"tail -200 /var/adm/syslog/syslog.log\"	tail	0.01\n"
"\"tail -200 /var/log/mail.log\"		tail	0.01\n"
"#\"tail -200 /var/adm/maillog\"		tail	0.01\n"
"#\"tail -200 /var/adm/syslog/mail.log\"	tail	0.01\n";

#endif /* _PRNG_CMDS_H_ */


