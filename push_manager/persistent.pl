package Embed::Persistent;
 # Preload scripts:

 use lib 'kn/cgi-bin';
 use KN::EventFormat;
 use KN::Error;
 use Data::Dumper;
 use Cwd;
 
 # This voodoo needed to get around symlink silliness...
 my $foo = getcwd;
 chdir 'kn/cgi-bin';
 &eval_file ('kn.cgi', 1);
 chdir $foo;

 #persistent.pl

 # force load frequent packages so they persist
 # packages loaded in cgi scripts are discarded 
 # as that occurs in a forked child of thttpd
 # and not in the main prog. ideally, we should
 # be able to exec perl in the same process as
 # thttpd but that would involve sorting out
 # IO redirects and cgi-kills for overrunning 
 # processes
 #--------------------------------------
 #use DBI;  
 use IO::Handle;

 #--------------------------------------

 # actually used packages
 use strict;
 use vars '%Cache';
 use Symbol qw(delete_package);
 use Cwd;
 use File::Spec;

 sub valid_package_name {
     my($string) = @_;
     $string =~ s/([^A-Za-z0-9\/])/sprintf("_%2x",unpack("C",$1))/eg;
     # second pass only for words starting with a digit
     $string =~ s|/(\d)|sprintf("/_%2x",unpack("C",$1))|eg;

     # Dress it up as a real package name
     $string =~ s|/|::|g;
     return "Embed" . $string;
 }

 sub eval_file {
     my($filename, $startup) = @_;

     $filename = File::Spec->catfile (getcwd, $filename);
     #open (TRACE, ">>/tmp/cgi-info.trace");
     
     my $package = valid_package_name($filename);
     my $mtime = -M $filename;
     if(defined ($Cache{$package}{mtime})
        &&
        $Cache{$package}{mtime} <= $mtime)
     {
        # we have compiled this subroutine already,
        # it has not been updated on disk, nothing left to do
        #print TRACE "already compiled $package handler\n";
     }
     else {
        local *FH;
        open FH, $filename or die "open '$filename' $!";
        local($/) = undef;
        my $sub = <FH>;
        close FH;

        #wrap the code into a subroutine inside our unique package
        my $eval = qq{package $package; sub handler { $sub; }};
        {
            # hide our variables within this block
            my($filename,$mtime,$package,$sub);
            eval $eval;
        }
        die $@ if $@;

        #cache it unless we're cleaning out each time
	#print TRACE "Setting mtime for $package to $mtime\n";
        $Cache{$package}{mtime} = $mtime; # unless $delete;
     }

     #close (TRACE);
     return if $startup;
     
     eval {$package->handler;};
     die $@ if $@;

     #take a look if you want
     #print Devel::Symdump->rnew($package)->as_string, $/;
 }

 1;

 __END__

