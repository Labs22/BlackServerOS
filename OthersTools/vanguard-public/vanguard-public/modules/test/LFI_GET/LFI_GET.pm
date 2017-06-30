package Vanguard::LFI_GET;
use base Vanguard::Module;
use strict;
use LW2;
use Vanguard::Vector;

sub new
{
    my $class = shift;
    my $self = {
	scan => shift,
	mod_tag => 'LFI_GET',
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

    my $lfi_test = $self->{config}->{lfi_test};
    my $lfi_match = $self->{config}->{lfi_match};
    my @lfi_exits = @{$self->{config}->{lfi_exits}};

    return if($vector->{response}->{whisker}->{data} =~ /$lfi_match/);

    foreach my $key (keys %{$vector->{data}->{get_params}}) {
	foreach my $exit (@lfi_exits) {

	    my $test_vector = $vector->clone();
	    $test_vector->{data}->{get_params}->{$key}->{value} = $lfi_test . $exit;

	    my $test_content = $self->api("WEBAPPS")->do_request($test_vector);

	    if($test_content =~ /$lfi_match/) {
		$vector->has_vuln({
		    key => $key,
		    lfi_test => $lfi_test,
		    exit => $exit,
				  });
		$vector->src_tree($self->{mod_tag});
		$self->api("CORE")->log_vuln($vector, "Discovered local file inclusion (LFI) vulnerability: $vector->{data}->{method} $vector->{data}->{path} -- URL PARAMETER $vector->{vuln}->{key}: included $vector->{vuln}->{lfi_test}$vector->{vuln}->{exit}");
	    }
	}
    }
}

1;
