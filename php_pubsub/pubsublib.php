<?php

# pubsublib.php - PubSub Client library for PHP

# Copyright 2003-2004 KnowNow, Inc.  All rights reserved.
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
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
# 
# 3. Neither the name of the KnowNow, Inc., nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# @KNOWNOW_LICENSE_END@
# 

$RCSID = '$Id: pubsublib.php,v 1.10 2004/04/19 05:39:15 bsittler Exp $';

if (! defined("PUBSUBLIB_PHP_INCLUDED"))
{
    define("PUBSUBLIB_PHP_INCLUDED", 1);

    ## CONSTANTS

    # We map unrecognized UTF sequences to this value.
    define('PUBSUB_ucsNoChar', 0xFFFD);

    # This is the maximum permitted UCS character value
    define('PUBSUB_ucsMaxChar', 0x7FFFFFFF);

    # This is the maximum permitted UTF-16/UCS-2 wyde value
    define('PUBSUB_ucs2max', 0xFFFF);

    # This is the maximum permitted UTF-16 character value
    define('PUBSUB_utf16max', 0x10FFFF);

    # we use the first characters of the UTF-16 surrogate ranges
    define('PUBSUB_utf16firstLowHalf', 0xD800);
    define('PUBSUB_utf16firstHighHalf', 0xDC00);

    # all UTF-16 surrogate pairs are offset by this value after decoding
    define('PUBSUB_utf16offset', 0x10000);

    # mask and shift applied to UTF-16 surrogate halves to extract useful values
    define('PUBSUB_utf16mask', 0x3FF);
    define('PUBSUB_utf16shift', 10);

    # mask and shift for UTF-8 continuation bytes
    define('PUBSUB_utf8mask', 0x3F);
    define('PUBSUB_utf8shift', 6);

    # pseudo-NULL
    unset($PUBSUB_NULL);
    define("PUBSUB_NULL", $PUBSUB_NULL);

    # OO-helpers for objects with mutable properties:
    # eval(PUBSUB_MUTABLE) at the beginning of a constructor;
    # eval(PUBSUB_IMMUTABLE) at the end of a destructor [if you have one];
    # eval(PUBSUB_MUTATE) at the beginning of any other method;
    # pubsub_setMutableValue($this, 'name', $value) sets the value of mutable property 'name';
    # pubsub_getMutableValue($this, 'name') returns the value of mutable property 'name'

    # detect PHP3 vs. PHP4 syntax
    if (5 == true)
    {
        # PHP4/PHP5: use a reference
        define("PUBSUB_MUTABLE",
               '$this->_pubsub_refs = array("this" => & $this);' .
               '$this->_pubsub_mutable = array();');
        define("PUBSUB_MUTATE",
               '$this = & $this->_pubsub_refs["this"];');
        define("PUBSUB_IMMUTABLE",
               'unset ($this->_pubsub_refs["this"]);' .
               '$this->_pubsub_mutable = PUBSUB_NULL;');
        define("_PUBSUB_GET_MUTABLE_VALUE",
               '$ret = $o->_pubsub_mutable[$name];');
        define("_PUBSUB_SET_MUTABLE_VALUE",
               '$o->_pubsub_mutable[$name] = $value;');
    }
    else
    {
        # PHP3: use the squirrel table
        $_pubsub_squirrel_table = array();
        define("PUBSUB_MUTABLE",
               'global $_pubsub_squirrel_table;' .
               '$this->_pubsub_squirrel_id = uniqid("_pubsub_squirrel_");' .
               '$_pubsub_squirrel_table[$this->_pubsub_squirrel_id] = array();');
        define("PUBSUB_MUTATE",
               '');
        define("PUBSUB_IMMUTABLE",
               'global $_pubsub_squirrel_table;' .
               'unset ($_pubsub_squirrel_table[$this->_pubsub_squirrel_id]);');
        define("_PUBSUB_GET_MUTABLE_VALUE",
               'global $_pubsub_squirrel_table;' .
               '$ret = $_pubsub_squirrel_table[$o->_pubsub_squirrel_id][$name];');
        define("_PUBSUB_SET_MUTABLE_VALUE",
               'global $_pubsub_squirrel_table;' .
               '$_pubsub_squirrel_table[$o->_pubsub_squirrel_id][$name] = $value;');
    }

    function pubsub_getMutableValue(& $o, $name)
    {
        $ret = PUBSUB_NULL;
        eval(_PUBSUB_GET_MUTABLE_VALUE);
        return $ret;
    }

    function pubsub_setMutableValue(& $o, $name, $value)
    {
        eval(_PUBSUB_SET_MUTABLE_VALUE);
    }

    # get a parameter (named $name) from one of the sources (_GET,
    # _POST, and _COOKIE by default; fallback sources are used if the
    # primary sources don't exist) or from the same-named global if
    # those sources aren't available; returns $default otherwise
    function kn__gpc($name, $default = PUBSUB_NULL, $sources = PUBSUB_NULL,
                     $fallback_sources = PUBSUB_NULL)
    {
        $newstyle = false;
        if (! $sources)
        {
            $sources = array("_GET", "_POST", "_COOKIE");
        }
        # Round 1: Try PHP4-style variable collections
        while (list($idx, $set) = each($sources))
        {
            global $$set;
            if (isset($$set))
            {
                $newstyle = true;
                $tmp = $$set;
                if (isset($tmp[$name]))
                {
                    return $tmp[$name];
                }
            }
        }
        if ($newstyle)
        {
            return $default;
        }
        # Round 2: Try PHP3-style equivalents
        if (! $fallback_sources)
        {
            $fallback_sources = array();
            reset($sources);
            while (list($idx, $set) = each($sources))
            {
                if ($set == "_FILES")
                {
                    $set = "HTTP_POST_FILES";
                }
                else if ($set == "_SERVER" or
                         $set == "_GET" or
                         $set == "_POST" or
                         $set == "_COOKIE" or
                         $set == "_ENV" or
                         $set == "_SESSION")
                {
                    $set = "HTTP" . $set . "_VARS";
                }
                else
                {
                    continue;
                }
                $fallback_sources[count($fallback_sources)] = $set;
            }
        }
        $sources = $fallback_sources;
        while (list($idx, $set) = each($sources))
        {
            global $$set;
            if (isset($$set))
            {
                $newstyle = true;
                $tmp = $$set;
                if (isset($tmp[$name]))
                {
                    return $tmp[$name];
                }
            }
        }
        if ($newstyle)
        {
            return $default;
        }
        # Round 3: Try a plain global.
        global $$name;
        if (isset($$name))
        {
            return $$name;
        }
        return $default;
    }

    # invoke a closure $closure, with argument values from $args. $closure
    # may be a function pointer or an object; if the latter, it should
    # support $closure->__call($args[0], $args[1], ...); any other
    # $closure is simply eval()'ed [and may call return true, and may
    # refer to the $args array, and should include a "return" statement]
    function kn_apply($closure, $args)
    {
        $expr = 'return $closure';
        $args_expr = '(';
        $_ = array();
        reset($args);
        while (list($key, $value) = each($args))
        {
            if (count($_))
            {
                $args_expr .= ',';
            }
            $args_expr .=
                '$_[' . count($_) . ']';
            $_[] = $value;
        }
        $args_expr .= ');';
        if (is_object($closure))
        {
            $expr .= '->__call' . $args_expr;
        }
        else if (! function_exists($closure))
        {
            $expr = $closure;
        }
        else
        {
            $expr .= $args_expr;
        }
        # default return value of kn_apply() is "false"
        $ret = false;
        # detect PHP3 vs. PHP4 syntax
        if (5 == true)
        {
            # PHP4: return inside eval sets eval value
            eval('$ret = eval($expr);');
        }
        else
        {
            # PHP3: return inside eval returns from outer context
            eval($expr);
        }
        return $ret;
    }

    function kn_utf8decode($input,
                           $converter =
                           # the default converter produces 8-bit ISO-8859-1
                           'return ($args[0] <= 0xFF) ? chr($args[0]) : "?";')
    {
        # stringify the input
        $input = '' . $input;
        # overall result
        $s = '';
        # residual character value from earlier bytes of a multibyte sequence
        $code = 0;
        # minimum allowed character value for a multibyte sequence
        $mincode = 0;
        # number of continuation bytes expected
        $expected_bytes = 0;
        for ($i = 0; $i < strlen($input); $i ++)
        {
            $c = ord($input[$i]);

            # binary pattern 0xxx xxxx is unchanged
            if (($c >= 0) and ($c < 0x80))
            {
                if ($expected_bytes)
                    $s .= kn_apply($converter, array(PUBSUB_ucsNoChar));
                $expected_bytes = 0;
                $s .= kn_apply($converter, array($c));
            }
            # binary pattern 10xx xxxx continues a UTF-8 sequence
            else if (($c & 0xC0) == 0x80)
            {
                if ($expected_bytes)
                {
                    $code = ($code << PUBSUB_utf8shift) | ($c & PUBSUB_utf8mask);
                    $expected_bytes = $expected_bytes - 1;
                    if (! $expected_bytes)
                    {
                        # overlong sequences get squashed here
                        if ($code < $mincode)
                            $code = PUBSUB_ucsNoChar;
                        $s .= kn_apply($converter, array($code));
                    }
                }
                else
                    $s .= kn_apply($converter, array(PUBSUB_ucsNoChar));
            }
            # binary pattern 110x xxxx introduces a two-byte UTF-8 sequence
            else if (($c & 0xE0) == 0xC0)
            {
                if ($expected_bytes)
                    $s .= kn_apply($converter, array(PUBSUB_ucsNoChar));
                $mincode = 0x80;
                $expected_bytes = 1;
                $code = $c & (PUBSUB_utf8mask >> $expected_bytes);
            }
            # binary pattern 1110 xxxx introduces a three-byte UTF-8 sequence
            else if (($c & 0xF0) == 0xE0)
            {
                if ($expected_bytes)
                    $s .= kn_apply($converter, array(PUBSUB_ucsNoChar));
                $mincode = 0x800;
                $expected_bytes = 2;
                $code = $c & (PUBSUB_utf8mask >> $expected_bytes);
            }
            # binary pattern 1111 0xxx introduces a four-byte UTF-8 sequence
            else if (($c & 0xF8) == 0xF0)
            {
                if ($expected_bytes)
                    $s .= kn_apply($converter, array(PUBSUB_ucsNoChar));
                $mincode = 0x10000;
                $expected_bytes = 3;
                $code = $c & (PUBSUB_utf8mask >> $expected_bytes);
            }
            # binary pattern 1111 10xx introduces a five-byte UTF-8 sequence
            else if (($c & 0xFC) == 0xF8)
            {
                if ($expected_bytes)
                    $s .= kn_apply($converter, array(PUBSUB_ucsNoChar));
                $mincode = 0x200000;
                $expected_bytes = 4;
                $code = $c & (PUBSUB_utf8mask >> $expected_bytes);
            }
            # binary pattern 1111 110x introduces a six-byte UTF-8 sequence
            else if (($c & 0xFE) == 0xFC)
            {
                if ($expected_bytes)
                    $s .= kn_apply($converter, array(PUBSUB_ucsNoChar));
                $mincode = 0x4000000;
                $expected_bytes = 5;
                $code = $c & (PUBSUB_utf8mask >> $expected_bytes);
            }
            # something illegal this way comes...
            else
            {
                if ($expected_bytes)
                    $s .= kn_apply($converter, array(PUBSUB_ucsNoChar));
                $expected_bytes = 0;
                $s .= kn_apply($converter, array(PUBSUB_ucsNoChar));
            }
        }
        if ($expected_bytes)
            $s .= kn_apply($converter, array(PUBSUB_ucsNoChar));
        return '' . $s;
    }

    function kn_htmlEscape_converter($c)
    {
        if ($c >= PUBSUB_utf16firstLowHalf and
            $c <= (PUBSUB_utf16firstHighHalf | PUBSUB_utf16mask))
        {
            # reserved surrogate codepoint
            $c = PUBSUB_ucsNoChar;
        }
        if (($c < 32) or ($c > 126) or ($c == ord("'"))) return "&#$c;";
        else if ($c == ord('&')) return "&amp;";
        else if ($c == ord('"')) return "&quot;";
        else if ($c == ord('<')) return "&lt;";
        else if ($c == ord('>')) return "&gt;";
        else return chr($c);
    }

    # replace unsafe characters with character entities.
    # makes any string safe for doc.write() as an HTML attribute or textarea.
    # does UTF-8 decoding.
    function kn_htmlEscape($t)
    {
        return kn_utf8decode($t, "kn_htmlEscape_converter");
    }

    if (! isset($kn_topic))
    {
        $kn_topic_ = kn__gpc("kn_topic", PUBSUB_NULL, array("_GET"));
        if ($kn_topic_)
        {
            $kn_topic = $kn_topic_;
            unset($kn_topic_);
        }
    }

    if (! isset($kn_topic) and
        getenv("PATH_INFO"))
    {
        $kn_topic = getenv("PATH_INFO");
    }

    if (! isset($kn_userid))
    {
        $kn_userid =
            kn__gpc("kn_userid",
                    "guest",
                    array("_GET", "_COOKIE"));
    }

    if (! isset($kn_displayname))
    {
        $kn_displayname =
            kn__gpc("kn_displayname",
                    "Guest User",
                    array("_GET", "_COOKIE"));
    }

    if (! isset($kn_server))
    {
        $kn_server =
            kn__gpc("kn_server",
                    PUBSUB_NULL,
                    array("_GET", "_COOKIE"));
    }

    if ($kn_server == PUBSUB_NULL and
        getenv("SERVER_ADDR") and
        getenv("SERVER_PORT"))
    {
        $_kn_server_protocol = "http";
        if (getenv("SSL") == "ON" or
            getenv("SERVER_PORT") == "443")
        {
            $_kn_server_protocol = "https";
        }
        else if (getenv("SERVER_PROTOCOL"))
        {
            $_kn_server_protocol =
                split('/', getenv("SERVER_PROTOCOL"));
            $_kn_server_protocol =
                $_kn_server_protocol[0];
            if (defined("LC_CTYPE"))
            {
                $_kn_currentLocale = setlocale(LC_CTYPE, "0");
                setlocale(LC_CTYPE, "C");
                $_kn_server_protocol = strtolower($_kn_server_protocol);
                setlocale(LC_CTYPE, $_kn_currentLocale);
            }
            else
            {
                $_kn_currentLocale = setlocale("LC_CTYPE", "0");
                setlocale("LC_CTYPE", "C");
                $_kn_server_protocol = strtolower($_kn_server_protocol);
                setlocale("LC_CTYPE", $_kn_currentLocale);
            }
        }
        $_kn_server_host = getenv("SERVER_ADDR");
        #$_kn_server_host = "127.0.0.1";
        $_kn_server_vhost = getenv("HTTP_HOST");
        if ($_kn_server_vhost != "")
        {
            if (gethostbyname($_kn_server_vhost) == $_kn_server_host)
            {
                $_kn_server_host = $_kn_server_vhost;
            }
        }
        $_kn_server_port = getenv("SERVER_PORT");
        if ($_kn_server_protocol == "http" and
            $_kn_server_port == "80" or
            $_kn_server_protocol == "https" and
            $_kn_server_port == "443")
        {
            $_kn_server_hostport =
                $_kn_server_host;
        }
        else
        {
            $_kn_server_hostport =
                $_kn_server_host . ":" .
                $_kn_server_port;
        }
        $kn_server =
            $_kn_server_protocol .
            "://" .
            $_kn_server_hostport .
            "/kn";
    }

    function kn_isSequence($sequence, $checkClosure = false)
    {
        # return true iff $sequence is a sequential, zero-based,
        # numerically-indexed array and ($checkClosure != false) ?
        # $checkClosure($key, $value, $sequence) : true
        if (! is_array($sequence))
        {
            return false;
        }
        $seqnum = 0;
        $is_sequence = true;
        reset($sequence);
        while (list($key, $value) = each($sequence))
        {
            if (is_int($key) and
                '' . $key == '' . $seqnum and
                ($checkClosure == "" or
                 kn_apply($checkClosure,
                          array($key, $value, $sequence))))
            {
                $seqnum ++;
            }
            else
            {
                $is_sequence = false;
                break;
            }
        }
        if ($seqnum != count($sequence))
        {
            $is_sequence = false;
        }
        return $is_sequence;
    }

    function kn_isName($value)
    {
        # return true iff $value is a name
        return (is_string($value) or
                is_long($value) or
                is_double($value));
    }

    function kn_isSequence_NAMEVALUE($key, $value, $sequence)
    {
        # return true iff $value is a name-value pair
        return (is_array($value) and
                count($value) == 2 and
                kn_isName($value[0]));
    }

    function kn_isSequence_ALTERNATES($key, $value, $sequence)
    {
        # return true iff $value is a name or $key is odd or $key is the
        # last item in $sequence
        return ($key % 2 or
                $key == (count($sequence) - 1) or
                kn_isName($value));
    }

    function kn_canonicalizeMessage($message)
    {
        # We expect a message to be a map. This function converts a
        # message string, sequence of name-value pairs, sequence of
        # alternating names and values (optionally followed by a message
        # string), or object to a map, if needed.
        if (! isset($message))
        {
            $message = array( );
        }
        if (is_object($message))
        {
            $message = $message->scalar;
        }
        if (! is_array($message))
        {
            $message = array( "kn_payload" => $message );
        }
        $canonicalMessage = array();
        if (kn_isSequence($message, "kn_isSequence_NAMEVALUE"))
        {
            # message is a sequence of array($name, $value) pairs
            reset($message);
            while (list($pos, $elm) = each($message))
            {
                list($name, $value) = $elm;
                if (isset($canonicalMessage[$name]))
                {
                    $mergedValues = array();
                    $values = $canonicalMessage[$name];
                    if (! kn_isSequence($values))
                    {
                        # single $values
                        $values = array( $values );
                    }
                    reset($values);
                    while (list($name2, $value2) = each($values))
                    {
                        $mergedValues[] = $value2;
                    }
                    if (! kn_isSequence($value))
                    {
                        # single $value
                        $value = array( $value );
                    }
                    reset($value);
                    while (list($name2, $value2) = each($value))
                    {
                        $mergedValues[] = $value2;
                    }
                    $value = $mergedValues;
                }
                $canonicalMessage[$name] = $value;
            }
        }
        else if (kn_isSequence($message, "kn_isSequence_ALTERNATES"))
        {
            # message is a sequence of alternating $name and $value
            # elements, optionally ended by a message string
            reset($message);
            $name = "kn_payload";
            while (list($pos, $value) = each($message))
            {
                if ($pos % 2 or
                    $pos == (count($message) - 1))
                {
                    if (isset($canonicalMessage[$name]))
                    {
                        $mergedValues = array();
                        $values = $canonicalMessage[$name];
                        if (! kn_isSequence($values))
                        {
                            # single $values
                            $values = array( $values );
                        }
                        reset($values);
                        while (list($name2, $value2) = each($values))
                        {
                            $mergedValues[] = $value2;
                        }
                        if (! kn_isSequence($value))
                        {
                            # single $value
                            $value = array( $value );
                        }
                        reset($value);
                        while (list($name2, $value2) = each($value))
                        {
                            $mergedValues[] = $value2;
                        }
                        $value = $mergedValues;
                    }
                    $canonicalMessage[$name] = $value;
                    $name = "kn_payload";
                }
                else
                {
                    $name = $value;
                }
            }
        }
        else
        {
            # message is an array
            reset($message);
            while (list($name, $value) = each($message))
            {
                $canonicalMessage[$name] = $value;
            }
        }
        return $canonicalMessage;
    }

    function kn_encodeForm($message)
    {
        $html = '';
        $message = kn_canonicalizeMessage($message);
        reset($message);
        while (list($name, $values) = each($message))
        {
            if (! kn_isSequence($values))
            {
                $values = array($values);
            }
            reset($values);
            while (list($pos, $value) = each($values))
            {
                if (is_array($value))
                {
                    $value =
                        kn_encodeForm(kn_canonicalizeMessage($value));
                }
                if (strlen($html))
                {
                    $html .= ';';
                }
                $html .= rawurlencode($name) . "=" . rawurlencode($value);
            }
        }
        return $html;
    }

    function kn_isEqualTo($a, $b)
    {
        if (gettype($a) != gettype($b)) return false;
        if (!is_array($a) or !is_array($b))
        {
            if ($a == $b)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        $ka=array();
        while (list($k, $v) = each($a))
        {
            $ka[] = $k;
            if (! kn_isEqualTo($b[$k],$v))
                return false;
        }
        $kb=array();
        while (list($k, $v) = each($b))
        {
            if ($ka[count($kb)] != $k) return false;
            $kb[] = $k;
            if (! kn_isEqualTo($a[$k],$v))
                return false;
        }
        if (count($ka) != count($kb)) return false;
        return true;
    }

    function kn_toSource($a)
    {
        $str = '';
        $type = gettype($a);
        if ($type == "NULL")
        {
            return "NULL";
        }
        if ($type != "array")
        {
            if ($type == "boolean")
            {
                $a = $a ? "true" : "false";
            }
            else if ($type == "object" or
                     $type == "user function" or
                     $type == "resource")
            {
                $str .= '(' . $type . ')';
                $type = "string";
            }
            if ($type == "string")
            {
                $str .= '"' . addslashes($a) . '"';
            }
            else
            {
                $str .= $a;
            }
            return $str;
        }
        $str .= 'array(';
        $comma = false;
        $suppressKey = kn_isSequence($a);
        while (list($k, $v) = each($a))
        {
            if ($comma) $str .= ', ';
            if (! $suppressKey)
            {
                $str .= kn_toSource($k) .
                    '=>';
            }
            $str .= kn_toSource($v);
            $comma = true;
        }
        return $str . ')';
    }

    # invoke status handlers on optional handler object for status event e
    function kn_doStatus($e, $handler = "")
    {
        # $handler->onStatus($e) interface
        if (is_object($handler) and
            gettype($handler.onStatus) == "user function")
        {
            return $handler->onStatus($e);
        }
        $code = "";
        if (isset($e["status"]))
        {
            $code = $e["status"];
            $code = $code[0];
        }
        if (($code == "1") or
            ($code == "2") or
            ($code == "3"))
        {
            # $handler->onSuccess($e) interface
            if (is_object($handler) and
                gettype($handler.onSuccess) == "user function")
            {
                return $handler->onSuccess($e);
            }
        }
        else
        {
            # $handler->onError($e) interface
            if (is_object($handler) and
                gettype($handler.onError) == "user function")
            {
                return $handler->onError($e);
            }
        }
        # fall back to the callable interface
        return kn_apply($handler, array($e));
    }

    # PubSub Client

    class PubSubClient
    {
        # properties, getters and setters
        var $myServerURI;
        var $myUserID;
        var $myDisplayName;
        function PubSubClient()
        {
            global $kn_server;
            global $kn_userid;
            global $kn_displayname;
            $this->setServerURI($kn_server);
            $this->setUserID($kn_userid);
            $this->setDisplayName($kn_displayname);
        }
        function setServerURI($aServerURI)
        {
            $this->myServerURI = $aServerURI;
        }
        function getServerURI()
        {
            return $this->myServerURI;
        }
        function setUserID($aUserID)
        {
            $this->myUserID = $aUserID;
        }
        function getUserID()
        {
            return $this->myUserID;
        }
        function setDisplayName($aDisplayName)
        {
            $this->myDisplayName = $aDisplayName;
        }
        function getDisplayName()
        {
            return $this->myDisplayName;
        }
        # methods
        function canonicalizeTopic($topic)
        {
            # Creates an absolute URI for a topic relative to a base server URI.
            $url = parse_url($topic);
            if ($url["scheme"])
            {
                # Complete topic with protocol.
                return $topic;
            }
            else if ($topic[0] == "/")
            {
                # FIXME: in this case, uri is not really a relative URI. if
                # it were, /what/chat relative to server
                # http://10.10.10.30/kn/cgi-bin/kn.cgi would resolve to
                # http://10.10.10.30/what/chat, not (as it does)
                # http://10.10.10.30/kn/cgi-bin/kn.cgi/what/chat
                return $this->getServerURI() . $topic;
            }
            else
            {
                return $this->getServerURI() . "/" . $topic;
            }
        }
        function setMessageDefaults($message)
        {
            $copy =
                array('userid' => $this->getUserID(),
                      'displayname' => $this->getDisplayName(),
                      "kn_id" => md5(uniqid(rand())));
            while (list($key, $value) = each($message))
            {
                $copy[$key] = $value;
            }
            return $copy;
        }
        function pubsubrequest($request = "", $statusHandler = "")
        {
            $requestCanon = kn_canonicalizeMessage($request);
            $request = array('kn_response_format' => 'simple');
            while (list($key, $value) = each($requestCanon))
            {
                $request[$key] = $value;
            }
            $kn_server = $this->getServerURI();
            $statusEvent =
                array('status' => '500 Internal Server Error',
                      'content-type' => 'text/plain',
                      'The server encountered an unexpected condition which prevented it\n' .
                      'from fulfilling the request.');
            if (stristr($kn_server, "http://") != $kn_server and
                stristr($kn_server, "https://") != $kn_server)
            {
                $statusEvent["kn_payload"] =
                    "Unsupported scheme in PubSub Server URL.\n" .
                    "Only HTTP and HTTP/S URLs are supported.";
                $statusEvent["html_payload"] =
                    "<h1>Unsupported scheme in PubSub Server URL</h1>" .
                    "<p>Only HTTP and HTTP/S URLs are supported.</p>";
            }
            else
            {
                @$result =
                    fopen($kn_server .
                          "?" .
                          kn_encodeForm($request),
                          "rb");
                if (!$result)
                {
                    $statusEvent["kn_payload"] =
                        "Unable to publish.\n" .
                        "HTTP GET request failed.";
                    $statusEvent["html_payload"] =
                        "<h1>Unable to publish</h1>" .
                        "<p>HTTP GET request failed.</p>";
                }
                else
                {
                    $match = 0;
                    $content = "";
                    while (!feof ($result))
                    {
                        # FIXME: lines might be longer than this
                        $line = fgets($result, 1024);
                        $content .= $line;
                        if ($line == "\n")
                        {
                            break;
                        }
                        # FIXME: those chars could be encoded
                        $needle = "status: ";
                        if (strstr($line, $needle) == $line)
                        {
                            $statusEvent["status"] = substr($line, strlen($needle));
                            $statusEvent["kn_payload"] = "";
                            $match = 1;
                        }
                    }
                    while (!feof ($result))
                    {
                        $content .= fgets($result, 1024);
                    }
                    fclose($result);
                    if (! $match)
                    {
                        $statusEvent["kn_payload"] =
                            "Invalid Status Event.\n" .
                            "Could not parse the status event returned by the PubSub Server.";
                        $statusEvent["html_payload"] =
                            "<h1>Invalid Status Event</h1>" .
                            "<p>Could not parse the status event returned by the PubSub Server.</p>";
                    }
                }
            }
            kn_doStatus($statusEvent, $statusHandler);
        }
        function publish($kn_to = "/", $event = "", $statusHandler = "")
        {
            $event = kn_canonicalizeMessage($event);
            $request =
                $this->setMessageDefaults(array("do_method" => "notify",
                                                "kn_to" => $kn_to));
            while (list($key, $value) = each($event))
            {
                $request[$key] = $value;
            }
            $request["kn_to"] =
                $this->canonicalizeTopic($request["kn_to"]);
            $statusEvent =
                $this->pubsubrequest($request, $statusHandler);
            return $request["kn_id"];
        }
        function subscribe($kn_from, $kn_to, $options = "", $statusHandler = "")
        {
            if ($options == "")
            {
                $options = array();
            }
            $options = kn_canonicalizeMessage($options);
            $request =
                $this->setMessageDefaults(array("do_method" => "route",
                                                "kn_to" => $kn_to,
                                                "kn_from" => $kn_from));
            while (list($key, $value) = each($options))
            {
                $request[$key] = $value;
            }
            $request['kn_from'] =
                $this->canonicalizeTopic($request['kn_from']);
            if (! isset($request['kn_uri']))
            {
                $rid = $request['kn_from'] .
                    "/kn_routes/" .
                    rawurlencode($request['kn_id']);
                $request['kn_uri'] = $rid;
            }
            $statusEvent =
                $this->pubsubrequest($request, $statusHandler);
            return $request['kn_uri'];
        }
        function unsubscribe($rid)
        {
            $regs = array();
            if (! ereg('^(.*)/kn_routes/([^/]+)$', $rid, $regs))
            {
                $statusEvent =
                    array(
                          'status' => "400 Bad Request",
                          'kn_payload' =>
                          "Client will not delete a route without the magic '/kn_routes/' substring."
                          );
                kn_doStatus($statusEvent, $statusHandler);
                return;
            }
            $kn_from = $regs[1];
            $kn_id = rawurldecode($regs[2]);
            $request =
                $this->setMessageDefaults(array('do_method' => 'route',
                                                'kn_id' => $kn_id,
                                                'kn_uri' => $rid,
                                                'kn_from' => $kn_from,
                                                'kn_to' => '',
                                                'kn_expires' => '+5'
                                                ));
            $request['kn_from'] =
                $this->canonicalizeTopic($request['kn_from']);
            $this->pubsubrequest($request, $statusHandler);
        }
    }

    # create a default client instance; others can be constructed manually

    $kn = new PubSubClient;

    # these exist to appease old-school PHP programmers who do not like
    # method syntax

    function kn_pubsubrequest($request = "", $statusHandler = "")
    {
        global $kn;
        return $kn->pubsubrequest($request, $statusHandler);
    }

    function kn_publish($kn_to = "/", $event = "", $statusHandler = "")
    {
        global $kn;
        return $kn->publish($kn_to, $event, $statusHandler);
    }

    function kn_subscribe($kn_from, $kn_to, $options = "", $statusHandler = "")
    {
        global $kn;
        return $kn->subscribe($kn_from, $kn_to, $options, $statusHandler);
    }

    function kn_unsubscribe($rid)
    {
        global $kn;
        return $kn->unsubscribe($rid);
    }

    # main program... unless we are included from another script

    function kn_fileIsEqualTo($file1, $file2)
    {
        # FIXME: URL-wrapper "files" may need special handling...
        return
            kn_isEqualTo($file1, $file2) or
            kn_isEqualTo(stat($file1),
                         stat($file2));
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
            <title>PubSub Client library for PHP</title>
            <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
            </head>
            <h1 align="center">Client library for PHP</h1>
            <br />
            <p align="center"><em>. . . consistent end-to-end event notification,
            <br />now in <a href="http://www.php.net" target="_blank">PHP</a>,
            <br />thanks to <a
            href="http://mod-pubsub.sourceforge.net/"
            target="_blank">mod_pubsub</a> . . .</em></p>
            <br />
            <?php

        function _kn_unitTest()
        {
            ?>
                <h2>Unit Tests</h2>
                <?php

            class _kn_unitTestSuite
            {
                var $allTests;
                function _kn_unitTestSuite()
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
            $s = new _kn_unitTestSuite;

            # BEGINNING OF TESTS

            $s->defineTest("nop", "test_nop");
            function test_nop()
            {
            }

            $s->defineTest("object mutation", "test_object_mutation");
            function test_object_mutation()
            {
                class _test_object_mutationO
                {
                    var $id;
                    function _test_object_mutationO()
                    {
                        eval(PUBSUB_MUTABLE);
                    }
                    function destroy()
                    {
                        eval(PUBSUB_IMMUTABLE);
                    }
                    function init()
                    {
                        eval(PUBSUB_MUTATE);
                        $this->setId("UNINITIALIZED");
                    }
                    function setId($new_id)
                    {
                        eval(PUBSUB_MUTATE);
                        pubsub_setMutableValue($this, 'id', $new_id);
                    }
                    function getId()
                    {
                        eval(PUBSUB_MUTATE);
                        return pubsub_getMutableValue($this, 'id');
                    }
                }
                function _test_object_mutation_a1(& $o, $new_id)
                {
                    $o->setId($new_id);
                }
                $o = new _test_object_mutationO();
                $o->init();
                $original_id = uniqid("");
                $o->setId($original_id);
                $new_id = uniqid("");
                _test_object_mutation_a1($o, $new_id);
                $res = false;
                if ($o->getId() == $original_id)
                {
                    $res = "objects are immutable";
                }
                $o->destroy();
                return $res;
            }

            $s->defineTest("kn_apply", "test_kn_apply");
            function test_kn_apply()
            {
                class _test_kn_applyO
                {
                    function _test_kn_applyO()
                    {
                        eval(PUBSUB_MUTABLE);
                    }
                    function destroy()
                    {
                        eval(PUBSUB_IMMUTABLE);
                    }
                    function init()
                    {
                        eval(PUBSUB_MUTATE);
                    }
                    function setAcc($n)
                    {
                        eval(PUBSUB_MUTATE);
                        pubsub_setMutableValue($this, 'acc', $n);
                    }
                    function getAcc()
                    {
                        eval(PUBSUB_MUTATE);
                        return pubsub_getMutableValue($this, 'acc');
                    }
                    function __call($m)
                    {
                        eval(PUBSUB_MUTATE);
                        $this->setAcc($this->getAcc() * $m);
                        return $this->getAcc();
                    }
                }
                $o = new _test_kn_applyO;
                $o->init();
                $o->setAcc(1);
                function _test_kn_apply_a1($o, $a1 = 1)
                {
                    $o->setAcc($a1);
                    if ($a1 < 2)
                    {
                        $r = 1;
                    }
                    else
                    {
                        $rr = kn_apply("_test_kn_apply_a1", array($o, $a1 - 1));
                        $r = kn_apply($o, array($a1));
                    }
                    return $r;
                }
                function _test_kn_apply_a2($a1, $a2, $o)
                {
                    return kn_apply($a1, array($o, $a2));
                }
                $n = 9;
                $nfact = 9 * 8 * 7 * 6 * 5 * 4 * 3 * 2 * 1;
                $fact = kn_apply(
                                 'return kn_apply("_test_kn_apply_a2"' .
                                 ',array($args[1],$args[0],$args[2]));',
                                 array($n, "_test_kn_apply_a1", $o));
                $o->destroy();
                if ($fact == $nfact)
                {
                    return "";
                }
                else
                {
                    return "$n factorial calculated as $fact, should be $nfact";
                }
            }

            function latin1()
            {
                return "\0=3b+;gar&FOO=3B\x7F\x80\x84\xFF??????\n\r\t <b>\"'";
            }

            function htmlEscaped()
            {
                return ("&#0;=3b+;gar&amp;FOO=3B" .
                        "&#127;&#128;&#132;&#255;&#256;&#8482;" .
                        "&#65279;&#65533;&#65536;" .
                        "&#1114109;&#10;&#13;&#9; &lt;b&gt;&quot;&#39;");
            }

            function utf8encoded()
            {
                return ("\x00\x3D3b\x2B\x3Bgar\x26FOO\x3D3B" .
                        "\x7F\xC2\x80\xC2\x84\xC3\xBF\xC4\x80" .
                        "\xE2\x84\xA2\xEF\xBB\xBF\xEF\xBF\xBD\xF0\x90\x80\x80" .
                        "\xF4\x8F\xBF\xBD\x0A\x0D\x09\x20\x3Cb\x3E\x22\x27");
            }

            $s->defineTest("kn_utf8decode", "test_kn_utf8decode");
            function test_kn_utf8decode()
            {
                $input = utf8encoded();
                $expected = latin1();
                $output = kn_utf8decode($input);
                if ($output != $expected)
                {
                    return "expected \"" . rawurlencode($expected) .
                        "\", got \"" . rawurlencode($output) .
                        "\" for \"" . rawurlencode($input) . "\"";
                }
            }

            $s->defineTest("kn_htmlEscape", "test_kn_htmlEscape");
            function test_kn_htmlEscape()
            {
                $input = utf8encoded();
                $expected = htmlEscaped();
                $output = kn_htmlEscape($input);
                if ($output != $expected)
                {
                    return "expected \"" . htmlentities($expected) .
                        "\", got \"" . htmlentities($output) .
                        "\" for \"" . rawurlencode($input) . "\"";
                }
            }

            $s->defineTest("kn_isSequence", "test_kn_isSequence");
            function test_kn_isSequence()
            {
                $seq =
                    array(array(),
                          array(1),
                          array(1,2),
                          array(0=>1,2),
                          array(1,1=>2),
                          array("0"=>";;;"));
                while (list($idx, $val) = each($seq))
                {
                    if (! kn_isSequence($val))
                        return "wrong result for \$seq[$idx]";
                }
                class _test_kn_isSequenceO { }
                $notseq =
                    array('',
                          1,
                          new _test_kn_isSequenceO,
                          array(2.0=>'fs'),
                          array(0.0=>2,1.0=>1,"2e0"=>2),
                          array(-1=>2,1),
                          array(1=>2,1),
                          array("00"=>";;;"));
                while (list($idx, $val) = each($notseq))
                {
                    if (kn_isSequence($val))
                        return "wrong result for \$notseq[$idx]";
                }
            }

            $s->defineTest("kn_isSequence 2", "test_kn_isSequence_2");
            function test_kn_isSequence_2()
            {
                $seq =
                    array(array());
                while (list($idx, $val) = each($seq))
                {
                    if (! kn_isSequence($val, "return false;"))
                        return "wrong result for \$seq[$idx]";
                }
                class _test_kn_isSequence_2O { }
                $notseq =
                    array('',
                          array(1),
                          array(1,2),
                          array(0=>1,2),
                          array(1,1=>2),
                          array("0"=>";;;"),
                          1,
                          new _test_kn_isSequence_2O,
                          array(2.0=>'fs'),
                          array(0.0=>2,1.0=>1,"2e0"=>2),
                          array(-1=>2,1),
                          array(1=>2,1),
                          array("00"=>";;;"));
                while (list($idx, $val) = each($notseq))
                {
                    if (kn_isSequence($val, "return false;"))
                        return "wrong result for \$notseq[$idx]";
                }
            }

            $s->defineTest("kn_canonicalizeMessage and kn_encodeForm",
                           "test_kn_canonicalizeMessage_and_kn_encodeForm");
            function test_kn_canonicalizeMessage_and_kn_encodeForm()
            {
                $cases =
                    array(array(input=>'',
                                expected=>array('kn_payload'=>''),
                                encoded=>"kn_payload="),
                          array(input=>array(2),
                                expected=>array('kn_payload' => 2),
                                encoded=>"kn_payload=2"),
                          array(input=>array(''),
                                expected=>array('kn_payload' => ''),
                                encoded=>"kn_payload="),
                          array(input=>"payload string",
                                expected=>array('kn_payload' => "payload string"),
                                encoded=>"kn_payload=payload%20string"),
                          array(input=>'1',
                                expected=>array('kn_payload' => '1'),
                                encoded=>"kn_payload=1"),
                          array(input=>array(),
                                expected=>array(),
                                encoded=>''),
                          array(input=>array('n1','v1','n2','v2','n1','v3'),
                                expected=>array('n1'=>array('v1', 'v3'),
                                                'n2'=>'v2'),
                                encoded=>"n1=v1;n1=v3;n2=v2"),
                          array(input=>array('n1','v1',
                                             'n2','v2',
                                             'n1','v3',
                                             'n1',array('n11'=>'v11'),
                                             'kn_payload',array('pn1'=>'pv1',
                                                                'pn2'=>array('pn21'=>'pnv21')),
                                             'v4'),
                                expected=>array('n1'=>array('v1','v3',
                                                            array('n11'=>'v11')),
                                                'n2'=>'v2',
                                                'kn_payload'=>
                                                array(array("pn1"=>"pv1",
                                                            "pn2"=>array("pn21"=>"pnv21")),
                                                      "v4")),
                                encoded=>("n1=v1;n1=v3;n1=n11%3Dv11;n2=v2;" .
                                          "kn_payload=pn1%3Dpv1%3Bpn2%3Dpn21%253Dpnv21;" .
                                          "kn_payload=v4")),
                          array(input=>array('n1',array('v1','v2'),
                                             'n1',array('v0'),
                                             'n1','v3',
                                             'n1','',
                                             'n1',array(),
                                             array('x','y')),
                                expected=>array('n1'=>array('v1','v2','v0','v3',''),
                                                'kn_payload'=>array('x','y')),
                                encoded=>"n1=v1;n1=v2;n1=v0;n1=v3;n1=;kn_payload=x;kn_payload=y"));
                while (list($idx, $val) = each($cases))
                {
                    $input = $val["input"];
                    $expected = $val["expected"];

                    $output = kn_canonicalizeMessage($input);
                    $err = "";
                    if (! kn_isEqualTo($output, $expected))
                    {
                        $err = "1";
                    }
                    else
                    {
                        $input = $output;
                        $output = kn_canonicalizeMessage($input);
                        if (! kn_isEqualTo($output, $expected))
                        {
                            $err = "2 (loop)";
                        }
                        else
                        {
                            $expected = $val["encoded"];
                            $output = kn_encodeForm($input);
                            if (! kn_isEqualTo($output, $expected))
                            {
                                $err = "3 (encode canonical)";
                            }
                            else
                            {
                                $input = $val["input"];
                                $output = kn_encodeForm($input);
                                if (! kn_isEqualTo($output, $expected))
                                {
                                    $err = "3 (encode)";
                                }
                            }
                        }
                    }
                    if ($err)
                    {
                        return "$idx-$err: expected <br /><b>" .
                            htmlentities(kn_toSource($expected)) .
                            "</b>,<br /> but got output <br /><b>" .
                            htmlentities(kn_toSource($output)) .
                            "</b><br /> for input <br /><b>" .
                            htmlentities(kn_toSource($input)) . "</b><br />";
                    }
                }
            }

            # FIXME: we also need tests for GET, POST and COOKIE data
            $s->defineTest("kn__gpc", "test_kn__gpc");
            function test_kn__gpc()
            {
                $input = "PATH";
                $expected = getenv("PATH");

                $output = kn__gpc($input, false, array("_ENV", "_SERVER"));
                $err = "";
                if (! kn_isEqualTo($output, $expected))
                {
                    $err = "check environment";
                }
                else
                {
                    $input = "nonexistent-" . md5(uniqid(rand()));
                    $expected = PUBSUB_NULL;

                    $output = kn__gpc($input, PUBSUB_NULL, array("_ENV", "_SERVER"));
                    if (! kn_isEqualTo($output, $expected))
                    {
                        $err = "check environment (negative)";
                    }
                }
                if ($err)
                {
                    return "$err: expected <br /><b>" .
                        htmlentities(kn_toSource($expected)) .
                        "</b>,<br /> but got output <br /><b>" .
                        htmlentities(kn_toSource($output)) .
                        "</b><br /> for input <br /><b>" .
                        htmlentities(kn_toSource($input)) . "</b><br />";
                }
            }

            # END OF TESTS

            $s->runTests();
        }

        _kn_unitTest();

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

} # ! defined("PUBSUBLIB_PHP_INCLUDED")

?>
