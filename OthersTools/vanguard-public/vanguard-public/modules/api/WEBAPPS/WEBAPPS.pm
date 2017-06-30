package Vanguard::WEBAPPS;
use base Vanguard::API;
use strict;

sub new
{
    my $class = shift;
    my $self = {
	scan => shift,
	config => undef,
	cookie_jar => {},
    };

    bless $self, $class;
    return $self;
}

sub namespace
{
    return "WEBAPPS";
}

# API functions

sub do_request
{
    my $self = shift;
    my $vector = shift;

    my $get_params;
    if($vector->{data}->{rewrite}) {
	foreach my $key ( sort {$a<=>$b} keys %{$vector->{data}->{get_params}}) {
	    $get_params .= "$vector->{data}->{get_params}->{$key}->{value}/";
	}
    } else {
	foreach my $key (keys %{$vector->{data}->{get_params}}) {
	    $get_params .= "$key=$vector->{data}->{get_params}->{$key}->{value}&";
	}
	chop $get_params;
    }

    my $request = $vector->{data}->{request};

    $request->{whisker}->{anti_ids} = $self->{scan}->{config}->{evasion};

    $request->{whisker}->{uri} = $vector->{data}->{path} . '?' . $get_params;

    if($request->{whisker}->{method} eq 'POST') {
	my $post_params;
	foreach my $key (keys %{$vector->{data}->{post_params}}) {
	    $post_params .= "$key=$vector->{data}->{post_params}->{$key}->{value}&";
	}
	chop $post_params;

	$request->{whisker}->{data} = $post_params;
    }

    LW2::http_fixup_request($request);

    my %response;

    $self->{scan}->debug("WEBAPPS: $request->{whisker}->{method} $request->{whisker}->{host}?$request->{whisker}->{uri}");

    if(not LW2::http_do_request($request, \%response)) {
	$request->{data}->{response} = \%response;

	# Put the cookies in the jar
	LW2::cookie_read($self->{cookie_jar}, \%response);

	# Return the content since its most commonly used
	return $response{whisker}{data};
    } else {
	# ?
    }
}

sub store_cookies
{
    my $self = shift;
    my $response = shift;

    LW2::cookie_read($self->{cookie_jar}, $response);
}

sub get_cookies
{
    my $self = shift;
    my $vector = shift;

    my $request = $vector->{data}->{request};

    my @names = LW2::cookie_get_valid_names($self->{cookie_jar}, $self->{scan}->{host},
					    $vector->{data}->{path}, $request->{whisker}->{ssl});
    
    my $c = 0;
    foreach my $name (@names) {
	# value, domain, path, expire, secure
	my @elements = LW2::cookie_get($self->{cookie_jar}, $name);
	$vector->{data}->{cookies}->{$name} = \@elements; # make into an array FOR MAX SPEEEED
	$c++;
    }

    return $c; # return the number of cookies read
}

sub set_cookies
{
    my $self = shift;
    my $vector = shift;

    my $request = $vector->{data}->{request};

    my $req_jar = LW2::cookie_new_jar();

    foreach my $name (keys %{$vector->{data}->{cookies}}) {
	LW2::cookie_set($req_jar, $name, @{$vector->{data}->{cookies}->{$name}});
    }

    LW2::cookie_write($req_jar, $request);
}

1;
