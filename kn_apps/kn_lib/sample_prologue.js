// sample_prologue.js - prologue for JavaScript pubsub applications

// To use:
//
// 1. copy to prologue.js
//
// 2. uncomment and edit to suit your cross-domain configuration
//
// 3. edit applications to point to appropriate server for
//    do_method=... scripts

// NOTE: All pages in your applications must reside inside the same
// JavaScript security domain for this to work, e.g. 'apps.foo.com'
// and 'pubsub.foo.com' are both in the 'foo.com' domain, but
// 'apps.bar.com' is not and hence would need a different
// document.domain setting.

// TODO: This should be made to work with ports other than 80 while
// still supporting Netscape 4.x, but that requires additional
// browser-sniffing.

//document.domain = "foo.com";

if (! self.kn_server)
{
  //self.kn_server = "http://pubsub.foo.com/kn";
}
