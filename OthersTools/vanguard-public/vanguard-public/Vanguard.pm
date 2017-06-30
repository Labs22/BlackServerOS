#!/usr/local/bin/perl
package Vanguard;
use strict;
use Data::Dumper;
use Clone qw(clone);
use YAML qw(LoadFile);
use Vanguard::Vector;
use Vanguard::Queue;

sub new
{
    my $class = shift;
    my $self = {
	rootstring => shift,
	queue => undef,
	modules => undef,
	api => undef,
	config => undef,
	persistent => undef,
	listener => 5900,
	redundant => undef,
	busy => undef,
	outfile => undef,
	conf_path => undef,
	registry => undef,
	debug => undef,
    };

    $self->{queue} = Vanguard::Queue->new;

    $self->{config} = LoadFile("config.yml");

    bless $self, $class;
    return $self;
}

sub workers
{
    my $self = shift;
    $self->{workers} = shift if(@_);
    return $self->{workers};
}

sub outfile
{
    my $self = shift;
    my $path = shift;

    open(STDOUT, ">$path");
}

sub conf_path
{
    my $self = shift;

    if(@_) {
	$self->{conf_path} = shift;
	$self->{config} = LoadFile("$self->{conf_path}/config.yml");
    }
    return $self->{conf_path};
}

sub debug
{
    my $self = shift;

    if(@_) {
	$self->{debug} = shift;
    }
    return $self->{debug};
}

sub DESTROY
{
    my $self = shift;

    $self->{queue}->enqueue(undef); # undef kills the queue

    foreach my $module (@{$self->{persistent}}) {
	$module->{queue}->enqueue(undef);
    }
}

sub queue_vector
{
    my $self = shift;
    my $vector = shift;
    my $prio = shift;

    if($prio) {
	$self->{queue}->prio_enqueue($vector);
    } else {
	$self->{queue}->enqueue($vector);
    }
}

sub vgprint
{
    my $self = shift;
    my $msg = shift;
    
    $self->info($msg);
}

sub run
{
    my $self = shift;

    while(1) {
	my $vector = $self->{queue}->dequeue;

	if($vector == undef) {
	    $self->info("Scan complete");
	    return;
	}

	foreach my $module (@{$self->{modules}}) {
	    my $test_vector = $vector->clone();
	    if($module->is_target($vector)) {
		$module->act($test_vector);
	    }
	}
    }
}

sub load_module
{
    my $self = shift;
    my $module_path = shift; # TODO: eliminate the need for this
    my $module_name = shift;    

    my @mod_wl = @{$self->{config}->{module_whitelist}};

    # Bit of a hack
    my $c = 0;
    foreach my $mod (@mod_wl) {
	$c++ if $module_name eq $mod;
    }

    $c++ if not $self->{config}->{use_whitelist};

    return 2 if not $c;

    eval "require '$module_path'";

    if($@) {
	print $@;
	return 0;
    }
    
    $module_path =~ /(.*)\//;
    my $dir = $1;

    if(defined $self->{conf_path}) {
	$dir = "$self->{conf_path}/$module_name";
    }

    my $module = "Vanguard::$module_name"->new($self);

    if($module->can("act") && $module->can("is_target")) {	
	push (@{$self->{modules}}, $module);
	if($module->can("load_conf")) {
	    $module->load_conf("$dir/");
	}
	return 1;
    } elsif($module->can("namespace")) {
	$self->{api}->{$module->namespace()} = $module;
	if($module->can("load_conf")) {
	     $module->load_conf("$dir/");
	}
	return 1;
    }

    return 0;
}

sub scan
{
    my $self = shift;

    my $root_vector = new Vanguard::Vector;

    $root_vector->src_tree('root');
    $root_vector->data({
	rootstring => $self->{rootstring},
		       });
    $self->queue_vector($root_vector);
    $self->run();
    exit;
}

sub is_redundant
{
    my $self = shift;
    my $fp = shift; # fingerprint

    my $ret = 0;
    {
	$ret = $self->{redundant}->{$fp};
	$self->{redunddant}->{$fp} = 1;
    }

    return $ret;
}

sub get_listener
{
    my $self = shift;

    my $port = $self->{listener};
    $self->{listener}++;
    return $port;
}

sub log_vuln
{
    my $self = shift;
    my $vector = shift;
    my $str = shift;

    $self->vgprint($vector->src_tree() . ': ' . $vector->{data}->{host} . ': ' . $str);
    if($self->{config}->{verbose}) {
	$self->vgprint($vector->src_tree() . ': ' . $vector->{data}->{host} . ': ' . Dumper($vector));
    }

    $self->queue_vector($vector, 1);
}

sub registry_write
{
    my $self = shift;
    my $key = shift;
    my $value = shift;
    
    $self->{registry}->{$key} = $value;
}

sub registry_read
{
    my $self = shift;
    my $key = shift;

    return $self->{registery}->{$key};
}

sub debug
{
    my ($self, $msg) = @_;

    if($self->{debug}) {
	print _timestamp() . " DEBUG: $msg\n";
    }
}

sub info
{
    my ($self, $msg) = @_;

    print _timestamp() . " INFO: $msg\n";
}

sub warn
{
    my ($self, $msg) = @_;

    print _timestamp() . " WARN: $msg\n";
}

sub error
{
    my ($self, $msg) = @_;

    print _timestamp() . " ERROR: $msg\n";
}

sub _timestamp
{
    my ($sec, $min, $hour, $mday, $mon, $year,
        $wday, $yday, $isdst) = localtime time;

    return sprintf "%4d-%02d-%02d %02d:%02d:%02d",
    $year + 1900, $mon + 1, $mday, $hour, $min, $sec;
}

1;
