#!/bin/echo source

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

# $Id: runall.bash,v 1.5 2003/05/22 23:41:28 bsittler Exp $

#
#  1 - added grep $0 into the subsRunning line to ensure that we picked up
#       throughput_subs that are ours and not another users
#
EXPECTED_ARGS=1
E_BADARGS=-1
E_FILEDOESNTEXIST=-2
E_ULIMITLOW=-3
E_PUBUNDEF=-4
E_SUBUNDEF=-5

if [ $# -ne $EXPECTED_ARGS ]
then
  echo "Usage: $(basename "$0") iniFileName"
  exit $E_BADARGS
fi

if [ ! -f "$1" ]
then
  echo "Usage: $(basename "$0") iniFileName"
  echo "The file $1 doesn't exist"
  exit $E_FILEDOESNTEXIST
fi

INIFILE="$1"

if [ "$(ulimit -n)" != "unlimited" ]
then
  echo "WARNING! WARNING! WARNING!"
  echo $'\t'"number of open files handles  is set to $(ulimit -n)"
  echo $'\t'"You will not be able to run more than that many"
  echo $'\t'"subscribers and publishers. "
fi

if [ "$(env | grep "PUBTOPICURI" | wc -l)" -ne 1 ]
then
  echo "ERROR!! ERROR!! ERROR!!"
  echo "PUBTOPICURI is undefined.  Need it to know which PubSub Server"
  echo "and URI to use."
  exit $E_PUBUNDEF
fi

if [ "$(env | grep "SUBTOPICURI" | wc -l)" -ne 1 ]
then
  echo "ERROR!! ERROR!! ERROR!!"
  echo "SUBTOPICURI is undefined.  Need it to know which PubSub Server"
  echo "and URI to use."
  exit $E_SUBUNDEF
fi

if [ "$(which throughput_sub | grep "^no" | wc -l)" -eq 1 ]
then
  echo "ERROR!! ERROR!! ERROR!!"
  echo "Don't have a path set to be able to run throughput_sub"
  exit $E_SUBUNDEF
fi

#
#  Create the routes out of the topic and into the journal
#
outputdir="Results-$INIFILE"
rm -rf "$outputdir"
mkdir "$outputdir"
makeRoutesOutFileName="$outputdir/$INIFILE-mr.log"
echo "$makeRoutesOutFileName"
echo "Making routes based on $INIFILE file"
python makeroutes.py -f "$INIFILE" > "$makeRoutesOutFileName" 2>&1
errFound="$(grep "Successful" "$makeRoutesOutFileName" | wc -l)"
if [ "$errFound" -ne 1 ]
then
    echo "Error! Error! Error!"
    echo "Makeroutes encountered an error.  Look in $makeRoutesOutFileName.out"
    exit
fi

#
#  sub.bash fires off a throughput_sub process for every 1000 subscribers
#
echo "Creating tunnels to the PubSub Server"
bash sub.bash

echo "Counting the number of subscribers that are running"
num2expect="$(grep "^-numpublishers" "$INIFILE" | sed $'s/[ \t][ \t]*/\t/' | cut -f 2)"
echo "Expecting to see $num2expect entries in throughput_sub output"
liveTunnels="$(grep "All Subscribers connected" "$INIFILE"*sub*log | wc -l)"
while [ "$liveTunnels" -lt "$num2expect" ]
do
  sleep 5
  liveTunnels="$(grep "All Subscribers connected" "$INIFILE"*sub*log | wc -l)"
  echo " $(date) Are all subscribers connected: $liveTunnels"
done

echo "Publishing events"
pubOutFileName="$outputdir/$INIFILE-pub.log"
python pub.py -f "$INIFILE" > "$pubOutFileName" 2>&1

sleep 20
echo "Checking to see if subscribers are still running"
subsRunning="$(ps -ef | fgrep :"$testrun": | grep -v grep | wc -l )"

while [ "$subsRunning" -gt 0 ]
do
  sleep 20
  subsRunning="$(ps -ef | fgrep :"$testrun": | grep -v grep | wc -l )"
  echo "$(date)|Running throughput_process (subscribers)=$subsRunning"
done

# Wait for any file I/O to complete.
sleep 30

#
#  OK, so this is important.  If we want to run multiple tests back-to-back
# then we need to unsubscribe and undo what makeroutes did previously.
# Optimally it would be nice to whack the journals and see the memory
# free up as well.
#

mv "$INIFILE"*.out "$INIFILE"*.log "$outputdir"
allSubsFileName="$outputdir/allsubs.out"
cat "$outputdir"/*sub*timing*out > "$allSubsFileName"

echo "SUCCESS with $INIFILE config. Publishers and subscribers have finished.  "
tmpFileName="$(date | sed 's/ /-/g')".ini
resultsFileName="$outputdir/$tmpFileName"

echo "Subscriber results" > "$resultsFileName"
python analyze.py -f "$allSubsFileName"  >> "$resultsFileName"
echo "*** Publisher performance metrics follow" >> "$resultsFileName"

#
#  The pub.py script writes it's perf results into *pub*timing*.out file
# so this is hard coded down here - YUCK!!!
#
python analyze.py -f "$outputdir"/*pub*timing*.out >> "$resultsFileName"
echo "Results are in $resultsFileName"
