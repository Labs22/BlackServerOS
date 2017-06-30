#!/usr/bin/perl
use strict;
use Vanguard;
use Getopt::Std;

my %opts = ();
getopts("h:e:o:m:pvc:d", \%opts);

my $root         = $opts{h};
my $out_file     = $opts{o};
my $evasion      = $opts{e} || 0;
my $verbose      = $opts{v};
my $conf_path    = $opts{c};
my $debug        = $opts{d};

if($< != 0) {
    print STDERR "You must be root to run this.\n";
    exit 1;
}

usage() if not defined $root;

my $scan = Vanguard->new($root);

#$scan->vgprint("Scanning $root");

# Do this first. Always.
if($conf_path) {
    $scan->conf_path($conf_path);
}

if($debug) {
    $scan->debug(1);
}

if($out_file) {
    $scan->outfile($out_file);
}

if($evasion) {
    $scan->{config}->{evasion} = $evasion; # add an accessor?
}

if($verbose) {
    $scan->{config}->{verbose} = 1;
}

print STDERR "Loading modules\n";
load_modules("modules/api"); # API's should be loaded first
load_modules("modules/recon");
load_modules("modules/test");

print STDERR "Scanning\n";
$scan->scan();

sub usage
{
    print STDERR "Usage: $0 -h [host/root]\n";
    print STDERR "  -c [conf path] Load config files from an alternate path\n";
    print STDERR "  -e [evasion]   IDS evasion technique to use\n";
    print STDERR "  -o [outfile]   File to output results to\n";
    print STDERR "  -v             Verbose\n";
    print STDERR "\n";
    exit 1;
}

sub load_modules
{
    my $module_root = shift;

    opendir MOD_ROOT, $module_root or die "Cannot open $module_root: $!";
    my @module_dirs = readdir MOD_ROOT;
    closedir MOD_ROOT;
    foreach my $dir (@module_dirs) {
	next if($dir =~ /^\./); # Ignore . and .. (and any hidden directories)
	my $ret = $scan->load_module("$module_root/$dir/$dir.pm", $dir);
	if($ret == 1) {
	    print STDERR "Loaded $module_root/$dir/$dir.pm\n";
	} elsif($ret == 0) {
	    print STDERR "Error loading $module_root/$dir/$dir.pm\n";
	}
    }
}
