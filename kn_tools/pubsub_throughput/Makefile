# Copyright 2000-2003 KnowNow, Inc.  All Rights Reserved.

# @KNOWNOW_LICENSE_START@

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:

# 1. Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.

# 2. Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in
# the documentation and/or other materials provided with the
# distribution.

# 3. The name "KnowNow" is a trademark of KnowNow, Inc. and may not
# be used to endorse or promote any product without prior written
# permission from KnowNow, Inc.

# THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL KNOWNOW, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# @KNOWNOW_LICENSE_END@

# $Id: Makefile,v 1.1 2003/04/29 07:31:51 ifindkarma Exp $

.SILENT:

# LINUX
INCLUDE_PATH=/usr/include
LIB_PATH=/usr/lib
COMPILER=gcc 
LIBS=`curl-config --libs` -lpthread
STRIP=/usr/bin/strip

# Solaris
# Need -lsocket and -lnsl
#INCLUDE_PATH=/usr/local/include
#LIB_PATH=/usr/local/lib
#COMPILER=gcc 
#LIBS=-lcurl -ldl -lnsl -lsocket -lpthread
#STRIP=/usr/ccs/bin/strip

#ifdef CRYPTO
# LIBS=$(LIBS)-lssl -lcrypto 
#endif


all: throughput throughput_pub throughput_sub
        rm -f *.o

throughput:
        echo compiling throughput.c ...
        $(COMPILER) -I$(INCLUDE_PATH) -L$(LIB_PATH) throughput.c -o throughputs $(LIBS) 
        $(COMPILER) -g -I$(INCLUDE_PATH) -L$(LIB_PATH) throughput.c -o throughputd -DDEBUG $(LIBS)
        $(COMPILER) -g -I$(INCLUDE_PATH) -L$(LIB_PATH) throughput.c -o throughputt -DDEBUG $(LIBS)
        $(STRIP) throughputs throughputd throughputt
        rm -f throughput.o

throughput_pub:
        echo compiling throughput_pub.c ...
        $(COMPILER) -I$(INCLUDE_PATH) -L$(LIB_PATH) throughput_pub.c -o throughput_pubs $(LIBS) 
        $(COMPILER) -g -I$(INCLUDE_PATH) -L$(LIB_PATH) throughput_pub.c -o throughput_pubd -DDEBUG $(LIBS)
        $(COMPILER) -g -I$(INCLUDE_PATH) -L$(LIB_PATH) throughput.c -o throughput_pubt -DDEBUG $(LIBS)
        $(STRIP) throughput_pubs throughput_pubd throughput_pubt
        rm -f throughput_pub.o


throughput_sub:
        echo compiling throughput_sub.c ...
        $(COMPILER) -I$(INCLUDE_PATH) -L$(LIB_PATH) throughput_sub.c throughput_sub_main.c -o throughput_subs $(LIBS) 
        $(COMPILER) -g -I$(INCLUDE_PATH) -L$(LIB_PATH) throughput_sub.c throughput_sub_main.c -o throughput_subd -DDEBUG $(LIBS)
        $(COMPILER) -g -I$(INCLUDE_PATH) -L$(LIB_PATH) throughput_sub.c throughput_sub_main.c -o throughput_subt -DDEBUG $(LIBS)
        $(STRIP) throughput_subs throughput_subd throughput_subt
        rm -f throughput_sub.o

throughput_test:
        echo compiling throughput_test.c ...
        $(COMPILER) -I$(INCLUDE_PATH) -L$(LIB_PATH) throughput_test.c -o throughput_tests $(LIBS) 
        $(COMPILER) -g -I$(INCLUDE_PATH) -L$(LIB_PATH) throughput_test.c -o throughput_testd -DDEBUG $(LIBS)
        $(COMPILER) -g -I$(INCLUDE_PATH) -L$(LIB_PATH) throughput_test.c -o throughput_testt -DDEBUG $(LIBS)
        $(STRIP) throughput_tests throughput_testd throughput_testt
        rm -f throughput_test.o

stringcattest:
        $(COMPILER) -I$(INCLUDE_PATH9) $(LIB_PATH9) stringcattest.c -o stringcattest $(LIBS)



clean:
        rm -f throughputs throughputd throughputt *.o

