#!/usr/bin/perl -w

# Copyright 2000-2002 KnowNow, Inc.  All Rights Reserved.
#
# @KNOWNOW_LICENSE_START@
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 
# 1. Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in
# the documentation and/or other materials provided with the
# distribution.
# 
# 3. The name "KnowNow" is a trademark of KnowNow, Inc. and may not
# be used to endorse or promote any product without prior written
# permission from KnowNow, Inc.
# 
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
# 
# @KNOWNOW_LICENSE_END@
#
# $Id: xform_pgrspk.cgi,v 1.1 2002/11/07 07:07:59 troutgirl Exp $

use strict;
use CGI ':standard';

my $payload = param('kn_payload');

my $abbrevlist = <<'EOF';
# English text
pagerspeak,pgrspk
possible,psbl
yesterday,ystrdy
cannot,cant
meeting,mtg
developing,devng
engineerng,engg
Internet,Inet
instant message,IM
technology,tech
today,2day
tomorrow,2moro
to,2
too,2
for,4
ate,8
later,l8r
be,B
bee,B
see,C
jay,J
cue,Q
are,R
tee,T
tea,T
you,U
eggs,X
why,Y
# Numbers and symbols
and,&
at,@
money,$
star,*
plus,+
equals,=
equal,=
percent,%
zero,0
one,1
two,2
three,3
four,4
five,5
six,6
seven,7
eight,8
nine,9
ten,10
greater than,>
greater,>
less than,<
less,<
# Airports
Boston,BOS
Burbank,BUR
Chicago,ORD
Cincinatti,CVG
Dallas,DFW
Irvine,SNA
Kansas City,MCI
Las Vegas,LAS
Los Angeles,LAX
Montreal,YUL
New York,NYC
Oakland,OAK
Orange County,SNA
Salt Lake City,SLC
San Diego,SAN
San Francisco,SFO
San Jose,SJC
Seattle,SEA
Vancouver,YVR
Washington,WAS
Washington DC,WAS
#People
Rohit,RK
Khare,RK
Rohit Khare,RK
Adam Rifkin,AR
Adam,AR
Rifkin,AR
Strata,SRC
Chalup,SRC
Strata Rose,SRC
Strata Rose Chalup,SRC
Ben,BS
Ben Sittler,BS
Kragen,KS
Kragen Sittler,KS
Kragen Sitaker,KS
Mike,MD
MikeD,MD
Mike Dierken,MD
Xander,XB
Xander Blakely,XB
Alexander,XB
Alexander Blakely,XB
Robert Liao,RL
Jon Staenberg,JS
Nohl,NF
Brad Silverberg,BradSi
Bill Gates,BillG
Bill Joy,BillJ
Larry Ellison,LarryE
John Chambers,JC
Steve Case,SC
Jane,JM
Jane Macfarlane,JM
Tim Berners-Lee,TimBL
Tim Berners Lee,TimBL
Vint,VC
Vint Cerf,VC
Marc Andreessen,MarcA
Marc Andresen,MarcA
Marty,JMT
Marty Tenenbaum,JMT
Mark Baker,MB
John Criswick,JC
Danny Lewin,DL
Tom Leighton,TL
Yonald Chery,YC
Rajan Srikanth,RS
Srikanth,RS
Anil Godhwani,AG
#Companies
Akamai,AKAM
Adobe,ADBE
Amazon,AMZN
America Online,AOL
Apple,AAPL
Ariba,ARBA
Avantgo,AVNT
BEA,BEAS
BEA Systems,BEAS
Bowstreet,BOWS
Broadcom,BRCM
Broadvision,BVSN
Cisco,CSCO
CMGi,CMGI
Commerce One,CMRC
Critical Path,CPTH
DataChannel,DCHA
eBay,EBAY
Ericsson,ERICY
Exodus,EXDS
Google,GOOG
Handspring,HAND
InfoSpace,INSP
Inktomi,INKT
Intel,INTL
KnowNow,KN
Lucent,LU
Microsoft,MSFT
Microstrategy,MSTR
Metricom,MCOM
Motorola,MOT
Network Appliance,NTAP
NaviSite,NAVI
Network Solutions,VRSN
Nextel,NXTL
Nokia,NOK
Nortel,NT
Oracle,ORCL
Palm,PALM
Phone.Com,PHCM
Puma,PUMA
Qualcomm,QCOM
Red Hat,RHAT
Sun,SUNW
Sun Micro,SUNW
Sun Microsystems,SUNW
Talarian,TALR
Tibco,TIBX
Valicert,VLCT
VeriSign,VRSN
Vignette,VIGN
Vitria,VITR
Webmethods,WEBM
Worldcom,WCOM
Web Consortium,W3C
World Wide Web Consortium,W3C
World Wide Web,W3
Yahoo,YHOO
EOF

my @abbrevs = 
    sort { length $b->[0] <=> length $a->[0] } 
    map { [ split /,/ ] } 
    grep { !/^#/ }
    split /\n/, $abbrevlist;

print header("text/plain");

my $abbrev;
foreach $abbrev (@abbrevs) 
{
    $payload =~ s/\b$abbrev->[0]\b/$abbrev->[1]/ig;
}

for ($payload)
{
    s/\r//g;
    s/\n\n+/|/g;
    s/\n/ /g;
    s/  +/ /g;
}

print $payload;

# End of xform_pgrspk.cgi
