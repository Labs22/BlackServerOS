package Vanguard::Queue;
use strict;
use warnings;

sub new
{
    my ($class) = @_;
    my $self = {};
    bless $self, $class;
    $self->{queue} = ();
    return $self;
}

sub enqueue
{
    my ($self, $q) = @_;

    unshift @{$self->{queue}}, $q;
}

sub prio_enqueue
{
    my ($self, $q) = @_;

    push @{$self->{queue}}, $q;
}

sub dequeue
{
    my ($self) = @_;

    pop @{$self->{queue}};
}

1;
