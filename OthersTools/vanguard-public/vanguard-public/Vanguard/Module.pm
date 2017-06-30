package Vanguard::Module;
use strict;
use YAML qw(LoadFile);
use Clone;
use Vanguard::Vector;

use Exporter();
our @ISA    = qw(Exporter);
our @EXPORT = qw(is_target act load_conf);

sub is_target {}

sub act {}

sub load_conf
{
    my $self = shift;
    my $conf_dir = shift;

    if(-e "$conf_dir/conf.yml") {
	$self->{config} = LoadFile("$conf_dir/conf.yml");
    }
}

sub api
{
    my $self = shift;
    my $namespace = shift;

    if($namespace eq 'CORE') {
	return $self->{scan};
    }

    return $self->{scan}->{api}->{$namespace};
}
