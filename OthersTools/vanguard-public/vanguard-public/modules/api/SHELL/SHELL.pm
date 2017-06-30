# Should be modified to use a socket file or SSL
package Vanguard::SHELL;
use base Vanguard::API;
use strict;
use IO::Socket::INET;
use threads;

sub new
{
    my $class = shift;
    my $self = {
	scan => shift,
	config => undef,
    };

    bless $self, $class;
    return $self;
}

sub namespace
{
    return "SHELL";
}

sub create_listener
{
    my $self = shift;
    my $func = shift;
    my @rest = @_;

    my $port = $self->{scan}->get_listener();
    
    threads->create(
	sub {
	    $self->{scan}->busy();

	    my $sock = IO::Socket::INET->new(Listen    => 5,
					     LocalAddr => 'localhost',
					     LocalPort => $port,
					     Proto     => 'tcp',
					     ReuseAddr => 1) or die "Cannot create socket: $@";

	    while(my $conn = $sock->accept()) {
		$conn->autoflush(1);
		print $conn "Vanguard> ";
		while(<$conn>) {
		    chomp;
		    my @args = (@rest, $_);
		    if($_ eq "done") {
			threads->exit();		
		    }
		    my $resp = $func->(@args);
		    $resp =~ s/\\n/\n/g; # for LFI bullshit
		    chomp $resp;
		    $resp .= "\n";
		    print $conn $resp;
		    print $conn "Vanguard> ";
		}	    
		close $conn;
	    }
	})->detach;
    
    return $port;
}

1;
