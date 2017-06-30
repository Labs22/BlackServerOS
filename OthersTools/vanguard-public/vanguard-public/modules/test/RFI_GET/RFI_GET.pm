package Vanguard::RFI_GET;
use base Vanguard::Module;
use strict;
use LW2;
use Vanguard::Vector;

sub new
{
    my $class = shift;
    my $self = {
	scan => shift,
	mod_tag => 'RFI_GET',
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

    my $rfi_test = $self->{config}->{rfi_test};
    my $rfi_match = $self->{config}->{rfi_match};

    return if $vector->{data}->{response}->{whisker}->{data} =~ /$rfi_match/;

    foreach my $key (keys %{$vector->{data}->{get_params}}) {
        my $test_vector = $vector->clone();

        $test_vector->{data}->{get_param}->{$key}->{value} = $rfi_test;

	my $content = $self->api("WEBAPPS")->do_request($test_vector);

	if($content =~ /$rfi_match/) {
	    $vector->src_tree($self->{mod_tag});
	    $vector->has_vuln({
		key => $key,
			      });
	    $self->api("CORE")->log_vuln($vector, "Discovered remote file inclusion (RFI) vulnerability: $vector->{data}->{method} $vector->{data}->{path} -- URL PARAMETER $vector->{vuln}->{$key}");
	}
    }
}

1;

