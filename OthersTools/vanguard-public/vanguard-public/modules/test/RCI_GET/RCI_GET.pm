package Vanguard::RCI_GET;
use base Vanguard::Module;
use strict;
use LW2;
use Vanguard::Vector;

sub new
{
    my $class = shift;
    my $self = {
        scan => shift,
        mod_tag => 'RCI_GET',
    };
    bless $self, $class;

    return $self;
}

sub is_target
{
    my $self = shift;
    my $vector = shift;

    return 1 if($vector->src_tree() eq 'CRAWL' &&
		keys %{$vector->{data}->{get_params}});
    return 0;
}

sub act
{
    my $self = shift;
    my $vector = shift;

    my @entries = @{$self->{config}->{entries}};

    foreach my $key (keys %{$vector->{data}->{get_params}}) {
	foreach my $entry (@entries) {
	    my $test_vector = $vector->clone();
	    $test_vector->{data}->{get_params}->{$key}->{value} = $entry . 'id' . $entry;
	    my $content = $self->api("WEBAPPS")->do_request($test_vector);
	    if($content =~ /\) gid=/) {
		$vector->src_tree($self->{mod_tag});
		$vector->has_vuln({
		    key => $key,
		    entry => $entry,
				  });
		$self->api("CORE")->log_vuln($vector, "Discovered remote command injection (RCI) vulnerability: $vector->{data}->{method} $vector->{data}->{path} -- URL PARAMETER $key: $entry" . ' [command] ' . "$entry");
	    }
	}
    }
}

1;
