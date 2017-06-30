package Vanguard::Vector;
use Clone;

our @ISA = qw(Clone);

sub new
{
    my $class = shift;
    my $self = {
	src_tree => undef,
	data => undef,
	vuln => undef,
    };
    
    bless $self, $class;
    return $self;
}

sub src_tree
{
    my $self = shift;

    $self->{src_tree} = shift if (@_);
    return $self->{src_tree};
}

sub data
{
    my $self = shift;

    $self->{data} = shift if(@_);
    return $self->{data};
}

sub has_vuln
{
    my $self = shift;
    my $vuln = shift;

    $self->{vuln} = $vuln;
}

1;
