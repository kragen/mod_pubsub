<?php
# eventloop.php - Network Event Loop for PHP

# Copyright 2003 KnowNow, Inc.  All Rights Reserved.
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

if (! defined("EVENTLOOP_PHP_INCLUDED"))
{
    define("EVENTLOOP_PHP_INCLUDED", 1);

    include("pubsublib.php");

    # network event loop (requires PHP4 w/socket extension)
    class EventLoop
    {
        # internal properties
        var $_readsocks;
        var $_writesocks;
        var $_exceptsocks;
        var $_onReadable;
        var $_onWritable;
        var $_onException;
        var $_timerlist;
        var $_timerobjs;
        var $_timerfreelist;
        var $_running;

        # used for reference verification
        var $_refs;

        ## general methods

        # return current time as floating-point time in seconds since Thu
        # Jan 1 00:00:00 UTC 1970
        function getTime()
        {
            eval($this->_verify);
            $now = split(" ", microtime());
            return $now[0] + $now[1];
        }

        # return string representation for debugging
        function repr()
        {
            eval($this->_verify);
            $str =
                "EventLoop ($this) {\n" .
                "   _readsocks: " . kn_toSource($this->_readsocks) . "\n" .
                "   _writesocks: " . kn_toSource($this->_writesocks) . "\n" .
                "   _exceptsocks: " . kn_toSource($this->_exceptsocks) . "\n" .
                "   _onReadable: " . kn_toSource($this->_onReadable) . "\n" .
                "   _onWritable: " . kn_toSource($this->_onWritable) . "\n" .
                "   _onException: " . kn_toSource($this->_onException) . "\n" .
                "   _timerlist: " . kn_toSource($this->_timerlist) . "\n" .
                "   _timerobjs: " . kn_toSource($this->_timerobjs) . "\n" .
                "   _timerfreelist: " . kn_toSource($this->_timerfreelist) . "\n" .
                "   _running: " . kn_toSource($this->_running) . "\n" .
                "}";
            return $str;
        }

        ## owner methods

        # constructor
        function EventLoop()
        {
            # detect PHP3 vs. PHP4 syntax
            if (5 == true)
            {
                # protect against accidental copy-on-write
                eval('$this->_refs = array("this" => & $this);');
                $this->_verify = '$this = & $this->_refs["this"];';
            }
            else
            {
                $this->_verify = '';
            }
        }

        # initializer
        function init()
        {
            eval($this->_verify);
            $this->_readsocks = array();
            $this->_writesocks = array();
            $this->_exceptsocks = array();
            $this->_onReadable = array();
            $this->_onWritable = array();
            $this->_onException = array();
            $this->_timerlist = array();
            $this->_timerobjs = array(PUBSUB_NULL);
            $this->_timerfreelist = array();
            $this->_running = false;
        }

        # high-level loop interface

        # start, run until stopped, then restore initial is-running
        # flag; the supplied closure $onPoll($rv, ...) is run each
        # time the loop wakes up, after network events have been
        # processed but before timer events have been processed; $rv
        # is the return value from the poll() method. A non-zero
        # return value from $onPoll($rv, ...) will stop the loop
        # without processing timer events.
        function run($onPoll = '', $args = PUBSUB_NULL)
        {
            eval($this->_verify);
            $brunning = $this->getRunning();
            $this->setRunning();
            if (! $args)
            {
                $args = array();
            }
            while ($this->getRunning())
            {
                $rv = $this->poll($this->getTimeout());
                if (kn_apply($onPoll,
                             array_merge(array($rv), $args)))
                {
                    $this->clearRunning();
                }
                else
                {
                    $this->runTimeouts();
                }
            }
            $this->setRunning($brunning);
        }

        # set is-running flag
        function setRunning($brunning = true)
        {
            eval($this->_verify);
            $this->_running = $brunning;
        }

        # clear is-running flag
        function clearRunning()
        {
            eval($this->_verify);
            $this->setRunning(false);
        }

        # get is-running flag
        function getRunning()
        {
            eval($this->_verify);
            return $this->_running;
        }

        # timer events

        # return the delay until the next scheduled event
        function getTimeout()
        {
            eval($this->_verify);
            reset($this->_timerlist);
            while (list($slot_major, $major_list) = each($this->_timerlist))
            {
                reset($major_list);
                while (list($slot_minor, $minor_list) = each($major_list))
                {
                    $now = $this->getTime();
                    $now_major = intval(floor($now));
                    $now_minor = intval(floor(($now - floor($now)) * 1e6));
                    if ($now_major > $slot_major or
                        $now_major == $slot_major and $now_minor > $slot_minor)
                    {
                        $slot_major = 0;
                        $slot_minor = 0;
                    }
                    $slot = $slot_major + 1e-6 * $slot_minor;
                    $now = $now_major + 1e-6 * $now_minor;
                    return $slot - $now;
                }
            }
            return PUBSUB_NULL;
        }

        # dispatch any pending timer events
        function runTimeouts()
        {
            eval($this->_verify);
            $runlist = array();
            reset($this->_timerlist);
            while (list($slot_major, $major_list) = each($this->_timerlist))
            {
                $now = $this->getTime();
                $now_major = intval(floor($now));
                if ($slot_major > $now_major)
                {
                    return;
                }
                reset($major_list);
                while (list($slot_minor, $minor_list) = each($major_list))
                {
                    $now = $this->getTime();
                    $now_major = intval(floor($now));
                    $now_minor = intval(floor(($now - floor($now)) * 1e6));
                    if ($slot_major == $now_major and $slot_minor > $now_minor)
                    {
                        return;
                    }
                    reset($minor_list);
                    while (list($pos, $id) = each($minor_list))
                    {
                        $closure = $this->_timerobjs[$id][0];
                        $args = $this->_timerobjs[$id][3];
                        $this->clearTimeout($id);
                        kn_apply($closure, $args);
                        $this->runTimeouts();
                        return;
                    }
                }
            }
        }

        # network events

        # poll and process network events, waiting at most $fmaxdelay
        # seconds; returns < 0 on error, 0 for no events processed, > 0
        # for zero or more events processed
        function poll($fmaxdelay = PUBSUB_NULL)
        {
            eval($this->_verify);
            $fmaxdelay_minor = 0;
            if ($fmaxdelay)
            {
                $fmaxdelay_minor = intval(floor(($fmaxdelay -
                                                 floor($fmaxdelay)) * 1e6));
                $fmaxdelay = intval(floor($fmaxdelay));
            }
            $_readsocks = array();
            reset($this->_readsocks);
            while (list($pos, $sock) = each($this->_readsocks))
            {
                $_readsocks[$pos] = $sock;
            }
            $_writesocks = array();
            reset($this->_writesocks);
            while (list($pos, $sock) = each($this->_writesocks))
            {
                $_writesocks[$pos] = $sock;
            }
            $_exceptsocks = array();
            reset($this->_exceptsocks);
            while (list($pos, $sock) = each($this->_exceptsocks))
            {
                $_exceptsocks[$pos] = $sock;
            }
            $rv = @socket_select($_readsocks, $_writesocks, $_exceptsocks,
                                $fmaxdelay, $fmaxdelay_minor);
            if (kn_isEqualTo($rv, false))
            {
                if (socket_last_error() == SOCKET_EAGAIN or
                    socket_last_error() == SOCKET_EWOULDBLOCK or
                    socket_last_error() == SOCKET_EINPROGRESS)
                {
                    # don't worry about it */
                }
                else
                {
                    trigger_error("select failed: " .
                                  socket_strerror(socket_last_error()) . "\n",
                                  E_USER_WARNING);
                }
            }
            else if ($rv > 0)
            {
                reset($_readsocks);
                while (list($pos, $sock) = each($_readsocks))
                {
                    if (! $this->_onReadable[$sock] or
                        kn_apply($this->_onReadable[$sock][0],
                                 $this->_onReadable[$sock][1]))
                    {
                        $this->clearInterestOnReadable($sock);
                    }
                }
                reset($_writesocks);
                while (list($pos, $sock) = each($_writesocks))
                {
                    if (! $this->_onWritable[$sock] or
                        kn_apply($this->_onWritable[$sock][0],
                                 $this->_onWritable[$sock][1]))
                    {
                        $this->clearInterestOnWritable($sock);
                    }
                }
                reset($_exceptsocks);
                while (list($pos, $sock) = each($_exceptsocks))
                {
                    if (! $this->_onException[$sock] or
                        kn_apply($this->_onException[$sock][0],
                                 $this->_onException[$sock][1]))
                    {
                        $this->clearInterestOnException($sock);
                    }
                }
            }
            return $rv;
        }

        ## client methods

        # timer events

        # cancel timer corresponding to handle $id
        function clearTimeout($id)
        {
            eval($this->_verify);
            $ent = $this->_timerobjs[$id];
            if (! $ent)
            {
                trigger_error("EventLoop::clearTimeout() failed: timer handle not assigned\n",
                              E_USER_WARNING);
            }
            else
            {
                $this->_timerobjs[$id] = PUBSUB_NULL;
                array_push($this->_timerfreelist, $id);
                $slot_major = $ent[1];
                $slot_minor = $ent[2];
                $this->_timerlist[$slot_major][$slot_minor] =
                    array_diff($this->_timerlist[$slot_major][$slot_minor],
                               array($id));
                if (! count($this->_timerlist[$slot_major][$slot_minor]))
                {
                    $this->_timerlist[$slot_major] =
                        array_diff_assoc($this->_timerlist[$slot_major],
                                         array($slot_minor =>
                                               $this->_timerlist[$slot_major][$slot_minor]));
                }
                if (! count($this->_timerlist[$slot_major]))
                {
                    $this->_timerlist =
                        array_diff_assoc($this->_timerlist,
                                         array($slot_major =>
                                               $this->_timerlist[$slot_major]));
                }
            }
        }

        # allocate a new timer to run $fdelay seconds from now and return
        # its handle; the handler $closure(...) is a closure to run when the
        # timer event fires
        function setTimeout($closure, $fdelay, $args = PUBSUB_NULL)
        {
            eval($this->_verify);
            $timeout = $this->getTime() + $fdelay;
            $slot_major = intval(floor($timeout));
            $slot_minor = intval(floor(($timeout - floor($timeout)) * 1e6));
            $id = array_pop($this->_timerfreelist);
            if (! $id)
            {
                $id = count($this->_timerobjs);
            }
            if (! $args)
            {
                $args = array();
            }
            $this->_timerobjs[$id] =
                array($closure,
                      $slot_major,
                      $slot_minor,
                      $args);
            if (! array_key_exists($slot_major, $this->_timerlist))
            {
                $this->_timerlist[$slot_major] =
                    array();
                ksort($this->_timerlist);
            }
            if (! array_key_exists($slot_minor, $this->_timerlist[$slot_major]))
            {
                $this->_timerlist[$slot_major][$slot_minor] =
                    array();
                ksort($this->_timerlist[$slot_major]);
            }
            array_push($this->_timerlist[$slot_major][$slot_minor],
                       $id);
            return $id;
        }

        # network events

        # shorthand; see setInterestOnReadable, setInterestOnWritable, and
        # setInterestOnException for details
        function setInterest($sock, $onReadable, $onWritable, $onException,
                             $args = PUBSUB_NULL)
        {
            eval($this->_verify);
            $this->setInterestOnReadable($sock, $onReadable, $args);
            $this->setInterestOnWritable($sock, $onWritable, $args);
            $this->setInterestOnException($sock, $onException, $args);
        }
        function clearInterest($sock)
        {
            eval($this->_verify);
            $this->clearInterestOnReadable($sock);
            $this->clearInterestOnWritable($sock);
            $this->clearInterestOnException($sock);
        }

        # set socket read handler for $sock; $onReadable(...) is a
        # closure or PUBSUB_NULL; returning a non-zero value from the
        # closure clears that handler
        function setInterestOnReadable($sock, $onReadable, $args = PUBSUB_NULL)
        {
            eval($this->_verify);
            if (! $args) $args = array();
            $this->_readsocks =
                array_diff($this->_readsocks,
                           array($sock));
            if ($onReadable)
            {
                array_push($this->_readsocks, $sock);
                $this->_onReadable[$sock] =
                    array($onReadable, $args);
            }
            else
            {
                $this->_onReadable[$sock] =
                    PUBSUB_NULL;
                $this->_onReadable =
                    array_filter($this->_onReadable);
            }
        }
        function clearInterestOnReadable($sock)
        {
            eval($this->_verify);
            $this->setInterestOnReadable($sock, PUBSUB_NULL);
        }

        # set socket write handler for $sock; $onWritable(...) is a
        # closure or PUBSUB_NULL; returning a non-zero value from the
        # closure clears that handler
        function setInterestOnWritable($sock, $onWritable, $args = PUBSUB_NULL)
        {
            eval($this->_verify);
            if (! $args) $args = array();
            $this->_writesocks =
                array_diff($this->_writesocks,
                           array($sock));
            if ($onWritable)
            {
                array_push($this->_writesocks, $sock);
                $this->_onWritable[$sock] =
                    array($onWritable, $args);
            }
            else
            {
                $this->_onWritable[$sock] =
                    PUBSUB_NULL;
                $this->_onWritable =
                    array_filter($this->_onWritable);
            }
        }
        function clearInterestOnWritable($sock)
        {
            eval($this->_verify);
            $this->setInterestOnWritable($sock, PUBSUB_NULL);
        }

        # set socket exception handler for $sock; $onException(...)
        # is a closure or PUBSUB_NULL; returning a non-zero value from the
        # closure clears that handler
        function setInterestOnException($sock, $onException, $args = PUBSUB_NULL)
        {
            eval($this->_verify);
            if (! $args) $args = array();
            $this->_exceptsocks =
                array_diff($this->_exceptsocks,
                           array($sock));
            if ($onException)
            {
                array_push($this->_exceptsocks, $sock);
                $this->_onException[$sock] =
                    array($onException, $args);
            }
            else
            {
                $this->_onException[$sock] =
                    PUBSUB_NULL;
                $this->_onException =
                    array_filter($this->_onException);
            }
        }
        function clearInterestOnException($sock)
        {
            eval($this->_verify);
            $this->setInterestOnException($sock, PUBSUB_NULL);
        }
    }

    define("_WRAPONREADABLE_OMITTED", uniqid("_wrapOnReadable_omitted"));

    class WrapOnReadable
    {
        var $_inner;
        function WrapOnReadable(& $inner)
        {
            eval('$this->_inner = & $inner;');
        }
        function __call($arg0 = _WRAPONREADABLE_OMITTED,
                        $arg1 = _WRAPONREADABLE_OMITTED,
                        $arg2 = _WRAPONREADABLE_OMITTED,
                        $arg3 = _WRAPONREADABLE_OMITTED,
                        $arg4 = _WRAPONREADABLE_OMITTED,
                        $arg5 = _WRAPONREADABLE_OMITTED)
        {
            if (! kn_isEqualTo($arg5, _WRAPONREADABLE_OMITTED))
            {
                trigger_error("WrapOnReadable::__call() called with too many args\n",
                              E_USER_ERROR);
            }
            if (! kn_isEqualTo($arg4, _WRAPONREADABLE_OMITTED))
                return $this->_inner->onReadable($arg0, $arg1, $arg2, $arg3, $arg4);
            if (! kn_isEqualTo($arg3, _WRAPONREADABLE_OMITTED))
                return $this->_inner->onReadable($arg0, $arg1, $arg2, $arg3);
            if (! kn_isEqualTo($arg2, _WRAPONREADABLE_OMITTED))
                return $this->_inner->onReadable($arg0, $arg1, $arg2);
            if (! kn_isEqualTo($arg1, _WRAPONREADABLE_OMITTED))
                return $this->_inner->onReadable($arg0, $arg1);
            if (! kn_isEqualTo($arg0, _WRAPONREADABLE_OMITTED))
                return $this->_inner->onReadable($arg0);
            return $this->_inner->onReadable();
        }
    }

    define("_WRAPONWRITABLE_OMITTED", uniqid("_wrapOnWritable_omitted"));

    class WrapOnWritable
    {
        var $_inner;
        function WrapOnWritable(& $inner)
        {
            eval('$this->_inner = & $inner;');
        }
        function __call($arg0 = _WRAPONWRITABLE_OMITTED,
                        $arg1 = _WRAPONWRITABLE_OMITTED,
                        $arg2 = _WRAPONWRITABLE_OMITTED,
                        $arg3 = _WRAPONWRITABLE_OMITTED,
                        $arg4 = _WRAPONWRITABLE_OMITTED,
                        $arg5 = _WRAPONWRITABLE_OMITTED)
        {
            if (! kn_isEqualTo($arg5, _WRAPONWRITABLE_OMITTED))
            {
                trigger_error("WrapOnWritable::__call() called with too many args\n",
                              E_USER_ERROR);
            }
            if (! kn_isEqualTo($arg4, _WRAPONWRITABLE_OMITTED))
                return $this->_inner->onWritable($arg0, $arg1, $arg2, $arg3, $arg4);
            if (! kn_isEqualTo($arg3, _WRAPONWRITABLE_OMITTED))
                return $this->_inner->onWritable($arg0, $arg1, $arg2, $arg3);
            if (! kn_isEqualTo($arg2, _WRAPONWRITABLE_OMITTED))
                return $this->_inner->onWritable($arg0, $arg1, $arg2);
            if (! kn_isEqualTo($arg1, _WRAPONWRITABLE_OMITTED))
                return $this->_inner->onWritable($arg0, $arg1);
            if (! kn_isEqualTo($arg0, _WRAPONWRITABLE_OMITTED))
                return $this->_inner->onWritable($arg0);
            return $this->_inner->onWritable();
        }
    }

    define("_WRAPONEXCEPTION_OMITTED", uniqid("_wrapOnException_omitted"));

    class WrapOnException
    {
        var $_inner;
        function WrapOnException(& $inner)
        {
            eval('$this->_inner = & $inner;');
        }
        function __call($arg0 = _WRAPONEXCEPTION_OMITTED,
                        $arg1 = _WRAPONEXCEPTION_OMITTED,
                        $arg2 = _WRAPONEXCEPTION_OMITTED,
                        $arg3 = _WRAPONEXCEPTION_OMITTED,
                        $arg4 = _WRAPONEXCEPTION_OMITTED,
                        $arg5 = _WRAPONEXCEPTION_OMITTED)
        {
            if (! kn_isEqualTo($arg5, _WRAPONEXCEPTION_OMITTED))
            {
                trigger_error("WrapOnException::__call() called with too many args\n",
                              E_USER_ERROR);
            }
            if (! kn_isEqualTo($arg4, _WRAPONEXCEPTION_OMITTED))
                return $this->_inner->onException($arg0, $arg1, $arg2, $arg3, $arg4);
            if (! kn_isEqualTo($arg3, _WRAPONEXCEPTION_OMITTED))
                return $this->_inner->onException($arg0, $arg1, $arg2, $arg3);
            if (! kn_isEqualTo($arg2, _WRAPONEXCEPTION_OMITTED))
                return $this->_inner->onException($arg0, $arg1, $arg2);
            if (! kn_isEqualTo($arg1, _WRAPONEXCEPTION_OMITTED))
                return $this->_inner->onException($arg0, $arg1);
            if (! kn_isEqualTo($arg0, _WRAPONEXCEPTION_OMITTED))
                return $this->_inner->onException($arg0);
            return $this->_inner->onException();
        }
    }

    if (getenv("SCRIPT_FILENAME") and
        kn_fileIsEqualTo(__FILE__,
                         getenv("SCRIPT_FILENAME")))
    {
        header("Content-Type: text/html; charset=utf-8");

        ?><!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
            <html>
            <head>
            <?php

        if (kn__gpc("show_source", false, array("_GET")))
        {
            ?><title><?php echo kn_htmlEscape(__FILE__); ?></title><?php
            show_source(__FILE__);
            exit;
        }

        ?>
            <title>Network Event Loop for PHP</title>
            <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
            </head>
            <h1 align="center">Network Event Loop for PHP</h1>
            <br />
            <p>Not much here yet...</p>
            <br />
            <?php

        function _eventLoop_unitTest()
        {
            ?>
                <h2>Unit Tests</h2>
                <?php

            class _eventLoop_unitTestSuite
            {
                var $allTests;
                function _eventLoop_unitTestSuite()
                {
                    $this->$allTests = array();
                }
                function runTests()
                {
                    $array = $this->allTests;
                    ?><p>Running <?php
                    echo kn_htmlEscape(count($array));
                    ?> Test(s)...</p><?php
                    ?><ol><?php
                    while (list($name, $closure) = each($array))
                    {
                        ?><li><?php
                        echo "\n" . kn_htmlEscape($name) . ":\n";
                        flush();
                        $result = kn_apply($closure, array());
                        if (! $result)
                        {
                            print "OK";
                        }
                        else
                        {
                            print "FAILED, " . $result;
                        }
                        ?></li><?php
                    }
                    ?></ol><?php
                    ?><p>Done.</p><?php
                }
                function defineTest($name, $closure)
                {
                    $this->allTests[$name] = $closure;
                }
            }
            $s = new _eventLoop_unitTestSuite;

            # BEGINNING OF TESTS

            $s->defineTest("nop", "test_nop");
            function test_nop()
            {
            }

            $s->defineTest("socket extension", "test_socket_ext");
            function test_socket_ext()
            {
                if (! function_exists("socket_select"))
                {
                    return "function socket_select() not found";
                }
            }

            # END OF TESTS

            $s->runTests();
        }

        _eventLoop_unitTest();

        ?>
            <hr size="1" />
            <p align="right" style="margin-top: 0px">You may find the <a
            href="<?php echo kn_htmlEscape(kn__gpc('PHP_SELF', false, array('_SERVER'))) . '?show_source=1'; ?>"
            >PHP source for this application</a> instructive.<br />
            Current <a href="http://www.php.net/" target="_blank">PHP</a> version:
            <?php echo kn_htmlEscape(phpversion()); ?></p>
            </body>
            </html>
            <?php
    }

} # ! defined("EVENTLOOP_PHP_INCLUDED")

?>
