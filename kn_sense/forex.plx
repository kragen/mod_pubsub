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
# $Id: forex.plx,v 1.1 2002/11/07 07:09:27 troutgirl Exp $

use strict;
use IO::Socket;
use URI::URL;
use LWP::Simple 'get';
use LWP::UserAgent;
use HTTP::Request::Common 'POST';
# use lib '../cgi-bin';
# use PubSub::Client; # We no longer use PubSub client library in this sensor.

my $serveruri = shift (@ARGV) || 'http://localhost/kn/';
my $desttopic = '/what/currencies';

my $interval = 0;  # seconds
my $intrastockwait = 0;  # seconds
my $ua = LWP::UserAgent->new;
my $next_time = time + $interval;
my %old_data;
my @datum = ();
my $evcount = 0;
my $routes_need_updating = 0;

# Curriencies to monitor for changes

my %currencies;
%currencies = (
	"AFA" =>	"Afghanistan Afghani",
	"ADF" =>	"Andorran Franc",
	"AWG" =>	"Aruba Florin",
	"BSD" =>	"Bahamian Dollar",
	"BBD" =>	"Barbados Dollar",
	"BMD" =>	"Bermuda Dollar",
	"BWP" =>	"Botswana Pula",
	"BND" =>	"Brunei Dollar",
	"XAF" =>	"CFA Franc (BEAC)",
	"CVE" =>	"Cape Verde Escudo",
	"CNY" =>	"Chinese Yuan",
	"CRC" =>	"Costa Rica Colon",
	"CYP" =>	"Cyprus Pound",
	"DJF" =>	"Dijibouti Franc",
	"XCD" =>	"East Caribbean Dollar",
	"SVC" =>	"El Salvador Colon",
	"EUR" =>	"Euro",
	"FIM" =>	"Finnish Mark",
	"DEM" =>	"German Mark",
	"XAU" =>	"Gold Ounces",
	"GNF" =>	"Guinea Franc",
	"HNL" =>	"Honduras Lempira",
	"ISK" =>	"Iceland Krona",
	"IQD" =>	"Iraqi Dinar",
	"ITL" =>	"Italian Lira .L",
	"JOD" =>	"Jordanian Dinar",
	"KRW" =>	"Korean Won",
	"LVL" =>	"Latvian Lat",
	"LRD" =>	"Liberian Dollar",
	"LUF" =>	"Luxembourg Franc",
	"MGF" =>	"Malagasy Franc",
	"MVR" =>	"Maldives Rufiyaa",
	"MUR" =>	"Mauritius Rupee",
	"MNT" =>	"Mongolian Tugrik",
	"MMK" =>	"Myanmar Kyat",
	"ANG" =>	"Neth Antilles Guilder",
	"NGN" =>	"Nigerian Naira",
	"OMR" =>	"Omani Rial",
	"XPD" =>	"Palladium Ounces",
	"PYG" =>	"Paraguayan Guarani",
	"XPT" =>	"Platinum Ounces",
	"QAR" =>	"Qatar Rial",
	"WST" =>	"Samoa Tala",
	"SCR" =>	"Seychelles Rupee",
	"SGD" =>	"Singapore Dollar",
	"SBD" =>	"Solomon Islands Dollar",
	"ESP" =>	"Spanish Peseta",
	"SDD" =>	"Sudanese Dinar",
	"SEK" =>	"Swedish Krona",
	"TWD" =>	"Taiwan Dollar",
	"TOP" =>	"Tonga Pa'anga",
	"TRL" =>	"Turkish Lira",
	"UGX" =>	"Ugandan Shilling",
	"VUV" =>	"Vanuatu Vatu",
	"YER" =>	"Yemen Riyal",
	"ZWD" =>	"Zimbabwe Dollar",
	"ALL" =>	"Albanian Lek",
	"ADP" =>	"Andorran Peseta",
	"AUD" =>	"Australian Dollar",
	"BHD" =>	"Bahraini Dinar",
	"BEF" =>	"Belgian Franc",
	"BTN" =>	"Bhutan Ngultrum",
	"BRL" =>	"Brazilian Real",
	"BIF" =>	"Burundi Franc",
	"KHR" =>	"Cambodia Riel",
	"KYD" =>	"Cayman Islands Dollar",
	"COP" =>	"Colombian Peso",
	"HRK" =>	"Croatian Kuna",
	"CZK" =>	"Czech Koruna",
	"DOP" =>	"Dominican Peso",
	"ECS" =>	"Ecuadorian Sucre",
	"EEK" =>	"Estonian Kroon",
	"FKP" =>	"Falkland Islands Pound",
	"FRF" =>	"French Franc",
	"GHC" =>	"Ghanian Cedi",
	"GRD" =>	"Greek Drachma",
	"GYD" =>	"Guyana Dollar",
	"HKD" =>	"Hong Kong Dollar",
	"INR" =>	"Indian Rupee",
	"IEP" =>	"Irish Punt",
	"JMD" =>	"Jamaican Dollar",
	"KZT" =>	"Kazakhstan Tenge",
	"KWD" =>	"Kuwaiti Dinar",
	"LBP" =>	"Lebanese Pound",
	"LYD" =>	"Libyan Dinar",
	"MOP" =>	"Macau Pataca",
	"MWK" =>	"Malawi Kwacha",
	"MTL" =>	"Maltese Lira",
	"MXN" =>	"Mexican Peso",
	"MAD" =>	"Moroccan Dirham",
	"NAD" =>	"Namibian Dollar",
	"NZD" =>	"New Zealand Dollar",
	"KPW" =>	"North Korean Won",
	"XPF" =>	"Pacific Franc",
	"PAB" =>	"Panama Balboa",
	"PEN" =>	"Peruvian Nuevo Sol",
	"PLN" =>	"Polish Zloty",
	"ROL" =>	"Romanian Leu",
	"STD" =>	"Sao Tome Dobra",
	"SLL" =>	"Sierra Leone Leone",
	"SKK" =>	"Slovak Koruna",
	"SOS" =>	"Somali Shilling",
	"LKR" =>	"Sri Lanka Rupee",
	"SRG" =>	"Surinam Guilder",
	"CHF" =>	"Swiss Franc",
	"TZS" =>	"Tanzanian Shilling",
	"TTD" =>	"Trinidad & Tobago Dollar",
	"USD" =>	"U.S. Dollar",
	"UAH" =>	"Ukraine Hryvnia",
	"VEB" =>	"Venezuelan Bolivar",
	"YUM" =>	"Yugoslav Dinar",
	"DZD" =>	"Algerian Dinar",
	"ARS" =>	"Argentine Peso",
	"ATS" =>	"Austrian Schilling",
	"BDT" =>	"Bangladesh Taka",
	"BZD" =>	"Belize Dollar",
	"BOB" =>	"Bolivian Boliviano",
	"GBP" =>	"British Pound",
	"XOF" =>	"CFA Franc (BCEAO)",
	"CAD" =>	"Canadian Dollar",
	"CLP" =>	"Chilean Peso",
	"KMF" =>	"Comoros Franc",
	"CUP" =>	"Cuban Peso",
	"DKK" =>	"Danish Krone",
	"NLG" =>	"Dutch Guilder",
	"EGP" =>	"Egyptian Pound",
	"ETB" =>	"Ethiopian Birr",
	"FJD" =>	"Fiji Dollar",
	"GMD" =>	"Gambian Dalasi",
	"GIP" =>	"Gibraltar Pound",
	"GTQ" =>	"Guatemala Quetzal",
	"HTG" =>	"Haiti Gourd",
	"HUF" =>	"Hungarian Forint",
	"IDR" =>	"Indonesian Rupiah",
	"ILS" =>	"Israeli Shekel",
	"JPY" =>	"Japanese Yen",
	"KES" =>	"Kenyan Shilling",
	"LAK" =>	"Lao Kip",
	"LSL" =>	"Lesotho Loti",
	"LTL" =>	"Lithuanian Lita",
	"MKD" =>	"Macedonian Denar",
	"MYR" =>	"Malaysian Ringgit",
	"MRO" =>	"Mauritania Ougulya",
	"MDL" =>	"Moldovan Leu",
	"MZM" =>	"Mozambique Metical",
	"NPR" =>	"Nepalese Rupee",
	"NIO" =>	"Nicaragua Cordoba",
	"NOK" =>	"Norwegian Krone",
	"PKR" =>	"Pakistani Rupee",
	"PGK" =>	"Papua New Guinea Kina",
	"PHP" =>	"Philippine Peso",
	"PTE" =>	"Portuguese Escudo",
	"RUB" =>	"Russian Rouble",
	"SAR" =>	"Saudi Arabian Riyal",
	"XAG" =>	"Silver Ounces",
	"SIT" =>	"Slovenian Tolar",
	"ZAR" =>	"South African Rand",
	"SHP" =>	"St Helena Pound",
	"SZL" =>	"Swaziland Lilageni",
	"SYP" =>	"Syrian Pound",
	"THB" =>	"Thai Baht",
	"TND" =>	"Tunisian Dinar",
	"AED" =>	"UAE Dirham",
	"UZS" =>	"Uzbekistan Sum",
	"VND" =>	"Vietnam Dong",
	"ZMK" =>	"Zambian Kwacha");
my @watch_list1 = sort(keys %currencies);
my @watch_list2 = sort(qw(USD EUR JPY GBP CHF));
my @watch_list3 = sort(qw(USD EUR));
my @watch_list = ();

# Add the dollar and euro and a few others to get things started.

@watch_list = sort(qw(EUR JPY GBP));
add_to_watchlist("USD");

# Watch for stock requests.

#my $serv = new PubSub::Client($serveruri);
#  $serv->subscribe("/what/currencies/kn_subtopics", sub {
#                       my ($event) = @_;
#                       add_to_watchlist($event->{kn_payload});
#                   }, { do_max_age => "infinity" })
#      || print "Failed to setup subscription to /what/currencies/kn_subtopics.\n";

sub add_to_watchlist {
    my ($s) = @_;
    my $sym = $s;
    $sym =~ tr[a-z][A-Z];
    if (defined $currencies{$sym}) {
      push @watch_list, $sym;
      print "Now monitoring \"$sym\" $currencies{$sym} on the fx market.\n";
      $routes_need_updating = 1;
    } else {
      print "\"$sym\" is not a recognized currency.\n";
    }
}

my $number_of_routes = 0;
sub calc_nor {
    my $i = 0;
    my $nr = @watch_list;
    for my $sc (@watch_list) {
        for my $dc (@watch_list) {
            if ($sc eq $dc) { next; }
            $i++;
        }
    }
    $i *= 2;
    $number_of_routes = $i;
}

# Ensure that routes we need exist
# This could use some optimization... GSB

sub construct_routes {
  my ($ssc, $sdc, $from, $to, @routes, $urlev, $rn);
  my $sofar = 0;
  my $nc = @watch_list;
  calc_nor;
  my $start_time = time;
  my $numev = 0;
  print "Creating $number_of_routes routes for $nc currencies.\n";
  for my $sc (@watch_list) {
    $ssc = $sc;
    $ssc =~ tr[A-Z][a-z];
    for my $dc (@watch_list) {
      if ($sc eq $dc) { next; }
      $sdc = $dc;
      $sdc =~ tr[A-Z][a-z];

      $from = "$desttopic/$ssc/rate/$sdc/spot";

      $to = "$desttopic/rate/spot";
      $rn = unpack("%32V*","$from $to");
      $urlev = "do_method=route&kn_from=$from&kn_to=$to&kn_id=$rn";
      push @routes, $urlev;

      $to = "$desttopic/$ssc/rate/spot";
      $rn = unpack("%32V*","$from $to");
      $urlev = "do_method=route&kn_from=$from&kn_to=$to&kn_id=$rn";
      $numev ++;
      push @routes, $urlev;
    }
    my $count = @routes;
    if ($count) {
        print "Establishing $count routes... ";
        my $req = POST $serveruri, [ do_method => 'batch', 
                                     kn_response_format => 'simple',
                                     map { (kn_batch => $_) } @routes ];
        my $result = $ua->request($req);
        if ($result->code !~ /^[123]/) {
            warn "Failed to post to $serveruri: " . $result->as_string;
        }
        $sofar += $count;
        my $pd = ($sofar / $number_of_routes) * 100;
        my $secsofar = (time - $start_time) || 1;
        my $rate = $numev / $secsofar;
        print " %" . $pd . " done ($rate" . "hz " . $secsofar . " " . $numev . ")\n";
        @routes = ();
    }
  }
}

sub post_datum {
    my ($src_currency, $dest_currency, $rate) = @_;

    my $smsrc_currency = $src_currency;
    $smsrc_currency =~ tr[A-Z][a-z];
    my $smdest_currency = $dest_currency;
    $smdest_currency =~ tr[A-Z][a-z];
    my $ex = "$src_currency/$dest_currency";
    my $dispex = "$currencies{$src_currency} ($src_currency)/$currencies{$dest_currency} ($dest_currency)";

    if (defined $rate and
#	(not defined $old_data{$ex} or$old_data{$ex} ne $rate) and
	$rate ne "0") {
        printf("The $currencies{$src_currency} will purchase $rate $currencies{$dest_currency}.\n");
	printf("(%8s\t%16s\t$desttopic/$smsrc_currency/rate/$smdest_currency/spot)\n", $ex, $rate);

        my $eid = unpack("%32V*","$desttopic $src_currency $dest_currency");
        my $when = time + 2048;
	my $urlev = "do_method=notify&kn_to=$desttopic/$smsrc_currency/rate/$smdest_currency/spot&displayname=$dispex&fromcurr=$currencies{$src_currency}&to_cursym=$dest_currency&from_cursym=$src_currency&tocurr=$currencies{$dest_currency}&type=spot&kn_payload=$rate&kn_expires=$when";
	push @datum, $urlev;

	$old_data{$ex} = $rate;
    }
}

sub post_all_data {
    if (@datum > 0) {
	my $req = POST $serveruri, [ do_method => 'batch', 
				 kn_response_format => 'simple',
				 map { (kn_batch => $_) } @datum ];
	my $result = $ua->request($req);
	if ($result->code !~ /^[123]/) {
	    warn "Failed to post to $serveruri: " . $result->as_string;
	}
	$evcount += @datum;
    }
    @datum=();
}

my %countries = (
		 'USD' => 'United States',
		 'EUR' => 'Euro',
		 'JPY' => 'Japan',
		 'GBP' => 'England',
		 'CHF' => 'Switzerland',
		 'RUB' => 'Russia'
		 );

sub make_fx_soap_req {
    my ($src_currency, $dest_currency) = @_;
    my $req = "";
    my $src_country = $countries{$src_currency};
    my $dest_country = $countries{$dest_currency};

#      $req = <<EOM
#  <?xml version="1.0"?>
#  <SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/">
#  <SOAP-ENV:Body>
#    <ns1:GetRate xmlns:ns1="http://www.itfinity.net/soap/exrates/exrates.xsd">
#    <fromCurr>$src_currency</fromCurr>      
#    <ToCurr>$dest_currency</ToCurr>
#  </ns1:GetRate>
#  </SOAP-ENV:Body>
#  </SOAP-ENV:Envelope>
#
#  EOM
#  ;
      $req = <<EOM
<?xml version='1.0' encoding='UTF-8'?>
<SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" xmlns:xsi="http://www.w3.org/1999/XMLSchema-instance" xmlns:xsd="http://www.w3.org/1999/XMLSchema">
<SOAP-ENV:Body>
<ns1:getRate xmlns:ns1="urn:xmethods-CurrencyExchange" SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/">
<country1 xsi:type="xsd:string">$src_country</country1>
<country2 xsi:type="xsd:string">$dest_country</country2>
</ns1:getRate>
</SOAP-ENV:Body>
</SOAP-ENV:Envelope>

EOM
;

    return $req;
}

sub soap_req {
  my ($soap_endpoint, $soap_action, $msg) = @_;
  my ($payload_length, $socket, $host_uri, $post_uri, $uri, $tmp, $len);
  my ($hostname, $port, $method, $response);

  $payload_length = length($msg);
  $uri = $soap_endpoint;
  $method = $soap_action;
  $soap_endpoint =~ m/^http:\/\/(\S+?)\/(\S+)/i;
  $host_uri = $1;
  ($hostname, $port) = split(':', $1);
  if(! $port ) {
    $port = 80;
  }
  $post_uri = "/" . $2;
  $socket = IO::Socket::INET->new(PeerAddr => $hostname,
				  PeerPort => $port,
				  Proto => "tcp",
				  Type => SOCK_STREAM) 
    or die "Couldn't open connection to $hostname:$port : $@\n";

  print $socket "POST $post_uri HTTP/1.0\r\n";
  print $socket "Host: $host_uri\r\n";
  print $socket "Accept: text/*\r\n";
  print $socket "Content-Type: text/xml\r\n";
  print $socket "Content-Length: $payload_length\r\n";
  print $socket "SOAPAction: $uri#$method\r\n";
  print $socket "\r\n";
  print $socket $msg;

  while( $_ = <$socket> ) {
    if($_ =~ m|content-length|i) {
      ($tmp, $len) = split(":", $_);
      ($len) = split(" ", $len);
    }
    if ( $_ =~ m/^\r?$/ ) {
      last;
    }
  }
  if (defined $len) { read($socket, $response, $len); }
  close($socket);
  return $response;
}

sub make_exchange_viaSOAP
{
  my($from,$to,$value)=@_;

  my $msg = make_fx_soap_req($from, $to);
#  my $result = soap_req("http://www.itfinity.net/soap/exrates/default.asp",
#			"GetQuote", $msg);
  my $result = soap_req("http://services.xmethods.net:80/soap",
			"getRate", $msg);
#  $result =~ /Rate = ([.0-9]+)/;
  $result =~ />([.0-9]+)</;
  my $rate = $1;
  return $rate;
}

sub make_exchange
  {
    my($from,$to,$value)=@_;

    $ua = new LWP::UserAgent;
    my $req = new HTTP::Request 'POST', 'http://www.xe.net/ucc/convert.cgi';
#    my $req = new HTTP::Request 'POST', 'http://216.220.38.20/ucc/convert.cgi';
    $req->content_type('application/x-www-form-urlencoded');
    my $curl = url("http:");
    my %form = ( Amount => $value, From => $from,  To => $to);
    $curl->query_form(%form);
    $req->content($curl->equery);
    print "req [http://www.xe.net/ucc/convert.cgi?Amount=$value&To=$to]\n";
    $_=$ua->request($req)->as_string;
    print "resp [$_]\n";
    if (/1 $from = ((\d*,)*\d*(\.\d*)?)\s*$to/i)
      {
	my $result = $1;
	if ($1 eq ",") {
	  return undef;
	}
	return $1;
      }

    return undef;
}

my $last_time = time;
my $cycle_time = 0;

for (;;)
{
  #$serv->handle_events(); # Let PubSub handle events.

  # If we are only watching one currency there is no need to get
  # exchange rate information.

  if (@watch_list <= 1) {
    sleep (2);
    next;
  }

  if ($routes_need_updating) {
      $routes_need_updating = 0;
      construct_routes();
  }

  for my $sc (@watch_list) {
    for my $dc (@watch_list) {
      if ($sc eq $dc) {
	next;
      }
      my $rate;
#      $rate = make_exchange($dc, $sc, 1);
#      printf("$rate\t%s\n", make_exchange_viaSOAP($dc, $sc, 1));
      if (!defined($rate)) {
	$rate = make_exchange_viaSOAP($dc, $sc, 1);
      }
      if (defined $rate) {
	post_datum($sc, $dc, $rate);
      }
      post_all_data(); # it takes too long to wait for all currencies...
      #$serv->handle_events(); # Let PubSub handle events.
      sleep (30);
    }
  }
  my $now = time;
  my $sleep_amount = $next_time - $now;
  if ($now < $next_time) {
#    sleep ($next_time - $now);
  }
  $next_time = time + $interval;
  $cycle_time = ($now - $last_time)
    - $sleep_amount
    - ($intrastockwait * @watch_list);
  $last_time = $now;
  print "------------------------------ (" .
    $cycle_time . " sec to update " . @watch_list .
    " symbols) [" . $evcount . "]\n";
}

# End of forex.plx
