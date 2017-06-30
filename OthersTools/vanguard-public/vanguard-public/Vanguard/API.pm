package Vanguard::API;
use strict;
use YAML qw(LoadFile);

use Exporter();
our @ISA    = qw(Exporter);
our @EXPORT = qw(namespace load_conf);

sub new
{
    my $class = shift;
    my $self = {
	config => undef,
    };

    bless $self, $class;
    return $self;
}

sub namespace {}

sub load_conf
{
    my $self = shift;
    my $conf_dir = shift;

    if(-e "$conf_dir/conf.yml") {
	$self->{config} = LoadFile("$conf_dir/conf.yml");
    }
}

1;
