package Vanguard::NMAP;
use base Vanguard::Module;
use strict;
use POSIX qw[ :sys_wait_h ];

sub new
{
    my $class = shift;
    my $self = {
	scan => shift,
	config => undef,
	mktrace => 0,
	mod_tag => "NMAP",
	mod_tree => "ports",
    };
    bless $self, $class;
    return $self;
}

sub is_target
{
    my $self = shift;
    my $vector = shift;

    return 1 if($vector->src_tree() eq 'root');
    return 0;
}

sub act
{
    my $self = shift;
    my $entry = shift;

    $self->{scan}->vgprint("$self->{mod_tag}: Scanning Ports...");

    $self->{scan}->vgprint("$self->{mod_tag}: nmap $self->{config}->{flags} -oG - $entry->{data}->{rootstring}");
    open(NMAP, "nmap $self->{config}->{flags} -oG - $entry->{data}->{rootstring} 2>/dev/null |");
    my @output = <NMAP>;
    close NMAP;

    foreach my $line (@output)
    {
	next if $line !~ /Host/;
	$line =~ /Host\: (\d+\.\d+\.\d+\.\d+)[^P]+Ports\:(.*)Ignored/g;
	#my $host     = $1;
	my $host     = $entry->{data}->{rootstring};
	my @services = split(',', $2);

	foreach my $service (@services)
	{
	    $service =~ /(\d+)\/(\w+)\/(\w+)\/\/(\w+)\/*/g;
	    my $port  = $1;
	    my $state = $2;
	    my $ip    = $3;
	    my $tcp   = $4;

	    my $vector = Vanguard::Vector->new();
	    $vector->src_tree($self->{mod_tag});
	    $vector->data({
		host => $host,
		port => $port,
		state => $state,
		protocol => $tcp,
			 });
	    $self->{scan}->queue_vector($vector);
	    if ($self->{mktrace} eq 0 && ($vector->{data}->{state} =~ /open/gi))
	    {
		my $trace_vector = Vanguard::Vector->new();
		$trace_vector->src_tree($self->{mod_tag});
		$trace_vector->data({
		    src_tree => 'network',
		    host => $host,
		    port => $port,
					});
		$self->{scan}->queue_vector($trace_vector);
		$self->{mktrace} = 1;
	    }

	    $self->{scan}->vgprint("$self->{mod_tag}: Port Scan found $tcp listener on $port in state $state");
	}
    }
}

1;
