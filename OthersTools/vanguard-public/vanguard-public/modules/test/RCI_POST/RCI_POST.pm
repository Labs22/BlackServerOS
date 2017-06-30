package Vanguard::RCI_POST;
use base Vanguard::Module;
use strict;
use LW2;
use Vanguard::Vector;

sub new
{
    my $class = shift;
    my $self = {
	scan => shift,
	mod_tag => 'RCI_POST',
    };

    bless $self, $class;
    return $self;
}

sub is_target
{
    my $self = shift;
    my $vector = shift;

    return 1 if($vector->src_tree() eq 'CRAWL' &&
		keys %{$vector->{data}->{post_params}});
    return 0;
}

sub act
{
    my $self = shift;
    my $vector = shift;

    my @entries = @{$self->{config}->{entries}};

    foreach my $key (keys %{$vector->{data}->{post_params}}) {
	foreach my $entry (@entries) {
	    my $test_vector = $vector->clone();
	    $test_vector->{data}->{post_params}->{$key}->{value} = $entry . 'id' . $entry;
	    my $content = $self->api("WEBAPPS")->do_request($test_vector);
	    if($content =~ /\) gid=/) {
		$vector->src_tree($self->{mod_tag});
		$vector->has_vuln({
		    key => $key,
		    entry => $entry,
				  });
		$self->api("CORE")->log_vuln($vector, "Discovered remote command injection (RCI) vulnerability: $vector->{data}->{method} $vector->{data}->{path} -- POST DATA $vector->{vuln}->{$key}: $entry" . 'command' . "$entry");
	    }
	}
    }
}

1;
