package PubSub::Htmlsafe;

# Copyright 2000-2004 KnowNow, Inc.  All rights reserved.
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
#
# $Id: Htmlsafe.pm,v 1.2 2004/04/19 05:39:09 bsittler Exp $

use strict;
use PubSub::UnitTest 'test_list', 'test';
use PubSub::UUID 'uuid';
use Exporter;
use base 'Exporter';

# Convert random HTML into fairly safe, if annoying, HTML.

# Uses a hand-rolled parser instead of an off-the-shelf, correct
# parser in order to exactly duplicate the behavior of the hand-rolled
# parser in our JavaScript chat client.

# We do this here so that people without JavaScript can get the same
# exciting benefits: big and small text, gigantic inline images, and
# hyperlinks.  Mostly hyperlinks.

# Accordingly, the shape of the code here follows the JavaScript
# code's shape (htmlsafe.js) rather closely.

@PubSub::Htmlsafe::EXPORT_OK = ('htmlSanitize', 'entify');

# Replace strange characters (including UTF-8 sequences) with
# equivalent SGML numeric character references, and (unless $ncronly
# is true) replace &, <, > and " with named character entities.

sub entify
{
    my ($string, $ncronly) = @_;
    $ncronly = 0 unless defined $ncronly;
    my $i;
    my $result = "";
    my $length = length $string;
    while (($length = length $string) > 0)
    {
        my $code = ord $string;
        my $ocode = $code;
        if ($code & 0x80)
        {
            # try to decode UTF-8 sequences
            if (($length >= 2) &&
                (($code & 0xE0) == 0xC0) &&
                ((0xC0 & ord substr $string, 1) == 0x80) &&
                (($ocode = 
                  (($code & 0x1F) << 6) |
                  (0x3F & ord substr $string, 1)) > 0x7F))
            {
                $string = substr $string, 1;
                $code = $ocode;
            }
            elsif (($length >= 3) &&
                   (($code & 0xF0) == 0xE0) &&
                   ((0xC0 & ord substr $string, 1) == 0x80) &&
                   ((0xC0 & ord substr $string, 2) == 0x80) &&
                   (($ocode =
                     (($code & 0xF) << 12) |
                     ((0x3F & ord substr $string, 1) << 6) |
                     (0x3F & ord substr $string, 2)) > 0x7FF))
            {
                $string = substr $string, 2;
                $code = $ocode;
            }
            elsif (($length >= 4) &&
                   (($code & 0xF8) == 0xF0) &&
                   ((0xC0 & ord substr $string, 1) == 0x80) &&
                   ((0xC0 & ord substr $string, 2) == 0x80) &&
                   ((0xC0 & ord substr $string, 3) == 0x80) &&
                   (($ocode =
                     (($code & 0x7) << 18) |
                     ((0x3F & ord substr $string, 1) << 12) |
                     ((0x3F & ord substr $string, 2) << 6) |
                     (0x3F & ord substr $string, 3)) > 0xFFFF))
            {
                $string = substr $string, 3;
                $code = $ocode;
            }
            elsif (($length >= 5) &&
                   (($code & 0xFC) == 0xF8) &&
                   ((0xC0 & ord substr $string, 1) == 0x80) &&
                   ((0xC0 & ord substr $string, 2) == 0x80) &&
                   ((0xC0 & ord substr $string, 3) == 0x80) &&
                   ((0xC0 & ord substr $string, 4) == 0x80) &&
                   (($ocode =
                     (($code & 0x3) << 24) |
                     ((0x3F & ord substr $string, 1) << 18) |
                     ((0x3F & ord substr $string, 2) << 12) |
                     ((0x3F & ord substr $string, 3) << 6) |
                     (0x3F & ord substr $string, 4)) > 0x1FFFFF))
            {
                $string = substr $string, 4;
                $code = $ocode;
            }
            elsif (($length >= 6) &&
                   (($code & 0xFE) == 0xFC) &&
                   ((0xC0 & ord substr $string, 1) == 0x80) &&
                   ((0xC0 & ord substr $string, 2) == 0x80) &&
                   ((0xC0 & ord substr $string, 3) == 0x80) &&
                   ((0xC0 & ord substr $string, 4) == 0x80) &&
                   ((0xC0 & ord substr $string, 5) == 0x80) &&
                   (($ocode =
                     (($code & 0x1) << 30) |
                     ((0x3F & ord substr $string, 1) << 24) |
                     ((0x3F & ord substr $string, 2) << 18) |
                     ((0x3F & ord substr $string, 3) << 12) |
                     ((0x3F & ord substr $string, 4) << 6) |
                     (0x3F & ord substr $string, 5)) > 0x3FFFFFF))
            {
                $string = substr $string, 5;
                $code = $ocode;
            }
        }
        if (($code == ord "&") && ! $ncronly)
        {
            $result .= "&amp;";
        }
        elsif (($code == ord "<") && ! $ncronly)
        {
            $result .= "&lt;";
        }
        elsif (($code == ord ">") && ! $ncronly)
        {
            $result .= "&gt;";
        }
        elsif (($code == ord "\"") && ! $ncronly)
        {
            $result .= "&quot;";
        }
        elsif (($code < 32) || ($code > 126))
        {
            $result .= "&#".$code.";";
        }
        else
        {
            $result .= chr $code;
        }
        $string = substr $string, 1;
    }
    return $result;
}

sub tokenize
{
    my ($tokenPatterns, $string) = @_;
    my @tokens = ();
    
  token: while ($string ne "")
  {
      for my $pat (@$tokenPatterns)
      {
      if ($string =~ m/$pat/g)
      {
          push @tokens, substr($string, 0, pos($string));
          $string = substr($string, pos($string));
          next token;
      }
      }
  }
    return @tokens;
}

# htmlemptytags should match HTML tags which don't allow content 
# (except for IMG tags, which have their own RegExp)

my $htmlemptytags =
    qr/^<[bh]r[ \r\n]*\/?>/i;

# htmlimgtags should match the IMG tag

my $htmlimgtags =
    qr/^<img([ \r\n]+src="(nntp:|news:|http:|https:|ftp:|gopher:)[^\000-\037 "<>]*"|[ \r\n]+alt="[^"<>]*")+[ \r\n]*\/?>/i;

# urlstrings should match "autohighlighted" URLs

my $urlstrings =
    qr/^(nntp|news|http|https|mailto|ftp|gopher):[^\000-\037 "<>]+/i;

# entities should match SGML character entities

my $entities = qr/^&(\#[0-9]+|\#x[0-9a-fA-F]+|[A-Za-z]+);/;

my $htmltokens = [
          $htmlemptytags,
          $htmlimgtags,
          $urlstrings,
          $entities,
          qr/^<[ \r\n]*\/[ \r\n]*>/i,
          qr/^<\/?(tt|i|b|big|small|strike|s|u|sub|sup)[ \r\n]*>/i,
          qr/^<\/?(em|strong|cite|dfn|code|samp|kbd|var|abbr|acronym)[ \r\n]*>/i,
          qr/^<(q|ins|del)([ \r\n]+cite=\"[^\"<>]*\")?[ \r\n]*>/i,
          qr/^<\/(q|ins|del)[ \r\n]*>/i,
          qr/^<a[ \r\n]+href=\"(nntp|news|http|https|mailto|ftp|gopher):[^\000-\037 \"<>]*\"[ \r\n]*>/i,
          qr/^<\/a[ \r\n]*>/i,
          qr/^(\r\n|\n\r)/,
          qr/^(.|[\r\n])/
         ];

sub htmlTagName
{
    my ($string) = @_;
    $string =~ s/^<\/?([a-zA-Z]+)[ \r\n>](.|[\r\n])*$/\L$1/;
    return $string;
}

sub isHtmlOpenTag
{
    my ($string) = @_;
    return $string =~ /^<[a-zA-Z]+([ \r\n]+[a-zA-Z]+=\"[^\">]*\")*[ \r\n]*\/?>$/;
}

sub isHtmlCloseTag
{
    my ($string) = @_;
    return $string =~ /^<\/[a-zA-Z]+[ \r\n]*>$/;
}

sub isHtmlCloseLastTag
{
    my ($string) = @_;
    return $string =~ /^<[ \r\n]*\/[ \r\n]*>$/;
}

sub htmlOpenTag
{
    my ($tagname) = @_;
    return "<$tagname>";
}

sub htmlCloseTag
{
    my ($tagname) = @_;
    return "</$tagname>";
}

sub htmlEmptyTag
{
    my ($tagname) = @_;
    return "<$tagname />";
}

sub closeElement
{
    my ($stack) = @_;
    return htmlCloseTag(pop @$stack);
}

sub findTagInStack
{
    my ($tag, $stack) = @_;
    
    for my $j (0..$#$stack)
    {
    if ($stack->[$j] eq $tag)
    {
        return $j;
    }
    }
    return undef;
}

sub htmlSanitize
{
    my ($string, $autohtml) = @_;
    if (not defined $string)
    {
    return $string;
    }
    
    my @tokens = tokenize($htmltokens, entify($string, 1));
    my @stack = ();
    for my $i (0..$#tokens)
    {
    if ($tokens[$i] eq "<")
    {
        $tokens[$i] = "&lt;";
    } 
    elsif ($tokens[$i] eq ">")
    {
        $tokens[$i] = "&gt;";
    }
    elsif ($tokens[$i] eq "&")
    {
        $tokens[$i] = "&amp;";
    }
    elsif ($tokens[$i] =~ $entities)
    {
        $tokens[$i] = "$tokens[$i]";
    }
    elsif ($autohtml && $tokens[$i] =~ $urlstrings)
    {
        if (!defined(findTagInStack('a', \@stack)))
        {
        $tokens[$i] = 
            "<a target=\"ChatLink" . uuid . "\" href=\"$tokens[$i]\">$tokens[$i]</a>";
        }
    }
    elsif (isHtmlCloseLastTag($tokens[$i]))
    {
        $tokens[$i] = '';
        if (@stack)
        {
        $tokens[$i] = closeElement(\@stack);
        }
    }
    elsif (isHtmlOpenTag($tokens[$i]))
    {
        my $token = $tokens[$i];
        $tokens[$i] = '';
        my $tag = htmlTagName($token);
        if ($token =~ $htmlemptytags)
        {
        $token = htmlEmptyTag($tag);
        if ($tag eq "br")
        {
            $token = "&nbsp;$token&nbsp;";
        }
        }
        elsif ($token =~ $htmlimgtags)
        {
        # passed verbatim
        }
        else
        {
        # "A" tags can't be nested.
        if ($tag eq "a")
        {
            my $j = findTagInStack($tag, \@stack);
            
            if (defined($j)) {
            while (@stack > $j)
            {
                $tokens[$i] .= closeElement(\@stack);
            }
            }
        }
        
        push @stack, $tag;
        if ($tag eq 'a')
        {
            my $uuid = uuid;

            $token =~ s/([ \r\n])/ target="ChatLink$uuid"$1/i;
        }
        else
        {
            $token = htmlOpenTag($tag);
        }
        }
        $tokens[$i] .= $token;
    }
    elsif (isHtmlCloseTag($tokens[$i]))
    {
        my $tag = htmlTagName($tokens[$i]);
        $tokens[$i] = '';
        while (@stack)
        {
        my $topTag = $stack[$#stack];
        
        $tokens[$i] .= closeElement(\@stack);
        if ($topTag eq $tag)
        {
            last;
        }
        }
    }
    else
    {
        $tokens[$i] =~ s/\r\n|\n\r/<br \/>/g;
        $tokens[$i] =~ s/[\r\n]/<br \/>/g;
    }
    }
    while (@stack)
    {
    push @tokens, closeElement(\@stack);
    }
    return join '', @tokens;
}

sub testHtmlTokens
{
    return tokenize $htmltokens, @_;
}


sub testTokenize
{
    test_list { testHtmlTokens "This is a test" }
    "basic tokenize", (split //, "This is a test");
    test_list { testHtmlTokens "<b>This</b> is a test" }
    "tag tokenize", ("<b>", (split //, "This"), "</b>", 
             (split //, " is a test"));
    test_list { testHtmlTokens "go to http://www.example.com/ and eat" }
    "URL tokenize", ((split //, "go to "), 
             "http://www.example.com/", 
             (split //, " and eat"));
    test_list { testHtmlTokens "a<hr>b" }
    "empty tag tokenize", ("a", "<hr>", "b");
    test_list { testHtmlTokens "x<hr />ml" }
    "empty XML tag tokenize", ("x", "<hr />", "m", "l");
    test_list { testHtmlTokens "n<br\n>l" }
    "tokenize empty tag with newline", 
    ("n", "<br\n>", "l");
    test_list { testHtmlTokens "<evil tag>" }
    "tokenize evil tag",
    (split //, "<evil tag>");
    test_list { testHtmlTokens '<img  src="http:trash">' }
    "tokenize img tag",
    ('<img  src="http:trash">');
    test_list { testHtmlTokens '<img alt="some trash" src="http:trash" >' }
    "tokenize image with alt",
    ('<img alt="some trash" src="http:trash" >');
    test_list { testHtmlTokens '<img alt=">bad stuff">' }
    "tokenize with bad alt",
    (split //, '<img alt=">bad stuff">');
    
    # It would be reasonable to only allow a single src attribute per
    # img tag, but allowing several currently makes this test and the
    # regexp it's testing a little easier to write.

    my $tag;
    $tag = '<img src="nntp:trash" src="news:evil" src="https:foo">';
    test_list { testHtmlTokens $tag }
    "tokenize img with nntp, news, and https",
    ($tag);
    $tag = '<img src="ftp:porn" src="gopher:silly">';
    test_list {testHtmlTokens $tag} "tokenize img with ftp and gopher", 
    ($tag);
    $tag = qq'<img src="http://badhost/\000">';
    test_list {testHtmlTokens $tag} "tokenize with null",
    (qw(< i ), 'm', 'g', ' ', 's', 
     qw(r c = ), '"', 'http://badhost/', "\000", '"', '>');
    $tag = '<img>';
    test_list {testHtmlTokens $tag} "tokenize img without attributes",
    (split //, $tag);
    $tag = '<iMg SrC="hTtP:blork">';
    test_list {testHtmlTokens $tag} "tokenize img with funny case",
    ($tag);
    $tag = '<img src="http:foo" />';
    test_list {testHtmlTokens $tag} "tokenize xml img", ($tag);
    
    my $urls = "nntp:foo news:bar http:baz https:plugh mailto:xyzzy ftp:glop";
    test_list {testHtmlTokens $urls} "tokenize with URLs", 
    (split /( )/, $urls);
    test_list {testHtmlTokens "gopher:matic"} "tokenize gopher URL",
    ("gopher:matic");
    my $url = "HtTP:gork";
    test_list {testHtmlTokens $url} "case-insensitive URL scheme",
    ($url);

    my $nasty_url = 'http://foo@bar.ba}z:80/~zorg/%7E@$,[$]^*\377?sth#sthelse';
    test_list {testHtmlTokens $nasty_url} "funny chars in URLs",
    ($nasty_url);
    my $illegal_urls = 
    "http://foo/>thing http://badhost/\000stuff http://badhost/\"stuff\"";
    test_list {testHtmlTokens $illegal_urls} "illegal URLs",
    ('http://foo/', (split //, '>thing '), 'http://badhost/', "\000",
     (split //, 'stuff '), 'http://badhost/', split //, '"stuff"');

    my $entities = "&#31; &#xf4; &#xF5; &amp;";
    test_list {testHtmlTokens $entities} "entity parsing",
    (split /( )/, $entities);
    my $bad_entities = "&#X77; &blort has no semicolon";
    test_list {testHtmlTokens $bad_entities} "bad entity parsing",
    (split //, $bad_entities);
    
    my $empty_close_tag = "</>";
    test_list {testHtmlTokens $empty_close_tag} "empty close tag",
    ($empty_close_tag);
    $empty_close_tag = "< \n\r/\r\n >";
    test_list {testHtmlTokens $empty_close_tag} 
    "empty close tag with whitespace",
    ($empty_close_tag);
    
    # tags without attributes
    my $lotsa_tags = "<tt><i ><b\r><big\n></small><strIke><s><u><sub><sup>";
    test_list {testHtmlTokens $lotsa_tags} "ordinary tags",
    ($lotsa_tags =~ m/(<(?:.|\n)*?>)/g);
    
    my $more_tags = "<em><strONg ><cite\r><dfn\n><code\r\n><samp></kbd></var>";
    test_list {testHtmlTokens $more_tags} "more ordinary tags",
    ($more_tags =~ m/(<(?:.|\n)*?>)/g);
    
    my $still_more_tags = '<abbr><acronym><q cite="stuff">' .
    qq'<ins\ncite="stuff" ><del cite="more stuff"></ins></q></del>' .
        "</a></a\r></a\n></a >";
    test_list {testHtmlTokens $still_more_tags} "fairly ordinary tags",
    ($still_more_tags =~ m/(<(?:.|\n)*?>)/g);
    
    my $broken_tag = '</ins cite="schemata">';
    test_list {testHtmlTokens $broken_tag} "close ins tag with attr",
    (split //, $broken_tag);
    
    my $anchor = '<a href="nntp://somewhere/">';
    test_list {testHtmlTokens $anchor} "simple anchor",
    ($anchor);
    
    my @schemes = qw(nntp news http https mailto ftp gopher);
    my $several_anchors = join '', map { qq(x<a href="$_:foo" \r\n>) } 
    @schemes;
    test_list {testHtmlTokens $several_anchors} "several anchors",
    (map { ('x', qq(<a href="$_:foo" \r\n>)) } @schemes);
    
    my $scary_anchor = qq(<a href="$nasty_url">);
    test_list {testHtmlTokens $scary_anchor} "scary anchor", $scary_anchor;
    
    my $gtanchor1 = '<a href="';
    my $gturl = 'http://badhost/';
    my $gtanchor2 = '>stuff">';
    test_list {testHtmlTokens "$gtanchor1$gturl$gtanchor2"} 
    "greater-than anchor", 
    ((split //, $gtanchor1), $gturl, (split //, $gtanchor2));
    
    my $bad_scheme_anchors = 
    '<a href="javascript:foo"> <a href="weird:stuff">';
    test_list {testHtmlTokens $bad_scheme_anchors} "bad scheme anchors",
    (split //, $bad_scheme_anchors);
    
    test_list {testHtmlTokens " \r \n \r\n \n\r  "} "whitespace",
    (' ', "\r", ' ', "\n", ' ', "\r\n", ' ', "\n\r", ' ', ' ');
}

sub testTagStuff
{
    test_list {htmlTagName '<foo>'} 'tag name', ('foo');
    test_list {htmlTagName '</a >'} 'end tag name', ('a');
    test {isHtmlOpenTag '<glork>'} 'start tag test';
    test {!isHtmlOpenTag '< glork>'} 'bad start tag test';
    test {isHtmlOpenTag '<a href="foo">'} 'start tag with attributes';
    test {isHtmlCloseTag '</a>'} 'close tag';
    test {isHtmlCloseTag "</a \r\n>"} 'close tag with whitespace';
    test {!isHtmlCloseTag '<a>'} 'not a close tag';
    test {isHtmlCloseLastTag '</>'} 'empty close tag';
    test {!isHtmlCloseLastTag '</x>'} 'not empty close tag';
    test_list {htmlOpenTag 'fork'} 'html open tag', '<fork>';
    test_list {htmlCloseTag 'fork'} 'html close tag', '</fork>';
    test_list {htmlEmptyTag 'fork'} 'html empty tag', '<fork />';
    test_list {
    my @x = qw(a bear of very little brain);
    my $tag = closeElement(\@x);
    ($tag, @x);
    } 'closeElement', qw(</brain> a bear of very little);
    test_list {
    my @x = 
    findTagInStack 'very', [qw(a bear of very little brain)];
    } 'findTagInStack', 3;
    test {
    !defined findTagInStack 'Pooh', [qw(a pear of very little rain)];
    } 'failure of findTagInStack';
}

sub testHtmlSanitize
{
    
}

sub testTests
{
    &test_list(sub { ('A', 'B', 'C'); }, "test_list test", qw(A B C));
    test_list { ('A', 'B', 'C'); } "test_list proto test", (qw(A B C));
}

sub selfTest
{
    testTests;
    testTokenize;
    testTagStuff;
    testHtmlSanitize;
}

1;

# End of Htmlsafe.pm
