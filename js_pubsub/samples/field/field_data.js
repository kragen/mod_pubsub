// Copyright 2001-2003 KnowNow, Inc.
 
// @KNOWNOW_LICENSE_START@

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in
// the documentation and/or other materials provided with the
// distribution.
// 
// 3. The name "KnowNow" is a trademark of KnowNow, Inc. and may not
// be used to endorse or promote any product without prior written
// permission from KnowNow, Inc.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL KNOWNOW, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
// IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// @KNOWNOW_LICENSE_END@

// $Id: field_data.js,v 1.1 2003/08/01 22:08:17 ifindkarma Exp $

people = new Array();

function Person(name,number,address) {
    this.name = name;
//  this.number = Math.floor(Math.random() * 100000);
    this.number = number;
    this.address = address;
    people[people.length] = this;
}

a = new Person("Jim Daniels",100,"103 Seville dr.");
b = new Person("Sally Brown",101,"P.O Box 29");
c = new Person("Brent McDowell",102,"349 Transit st.");
d = new Person("Fred Wolin",103,"2091 Main st.");
e = new Person("Robert Collins",104,"12 Oak ct.");

aa = new Person("Jim Daniels",1,"103 Seville dr.");
bb = new Person("Sally Brown",2,"P.O Box 29");
cc = new Person("Brent McDowell",3,"349 Transit st.");
dd = new Person("Fred Wolin",4,"2091 Main st.");
ee = new Person("Robert Collins",5,"12 Oak ct.");

ff = new Person("Robert Collins","555-1212","12 Oak ct.");

// End of field_data.js
