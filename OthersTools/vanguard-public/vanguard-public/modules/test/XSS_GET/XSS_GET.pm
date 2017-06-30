package Vanguard::XSS_GET;
use base Vanguard::Module;
use strict;
use LW2;
use Vanguard::Vector;

sub new
{
    my $class = shift;
    my $self = {
	scan => shift,
	mod_tag => 'XSS_GET',
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

    my @delims;
    for(0..3) {
	push @delims, generate_string(6);
    }
    my $tag    = '<' . generate_string(6) . '>';

    my $tag_match = $delims[0] . $tag;
    my $double_match = $delims[1] . '"';
    my $quote_match = $delims[2] . "'";

    # Randomize the order of the injections within the test string
    my $test_string;
    $test_string .= $_ foreach (sort { int(rand(3)) - 1 } ($tag_match, $double_match, $quote_match));

    $test_string .= $delims[3];

    foreach my $key (keys %{$vector->{data}->{get_params}}) {
	my $test_vector = $vector->clone();
	$test_vector->{data}->{get_params}->{$key}->{value} = $test_string;

	my $test_content = $self->api("WEBAPPS")->do_request($test_vector);

	my %states = (
	    "<" => sub {
		my $state = shift;
		$state->{in_tags} = 1 if((not $state->{in_quotes}) && (not $state->{in_double}));
	    },
	    ">" => sub {
		my $state = shift;
		$state->{in_tags} = 0 if((not $state->{in_quotes}) && (not $state->{in_double}));
	    },
	    "'" => sub {
		my $state = shift;
		$state->{in_quotes} = (not $state->{in_quotes}) if($state->{in_script});
	    },
	    '"' => sub {
		my $state = shift;
		$state->{in_double} = (not $state->{in_double}) if($state->{in_tags});
	    },
	    '<script>' => sub {
		my $state = shift;
		$state->{in_script} = 1;
	    },
	    '</script>' => sub {
		my $state = shift;
		$state->{in_script} = 0;
	    },
	    "$delims[0]$tag" => sub {
		my ($state, $vector) = @_;
		if((not $state->{in_double}) || (not $state->{in_quotes})) {
		    $vector->src_tree($self->{mod_tag});
		    $vector->has_vuln({
			key => $key,
			tags => 1,
				      });
		    $self->api("CORE")->log_vuln($vector, "Discovered cross-site scripting (XSS) vulnerability: $vector->{data}->{method} $vector->{data}->{path} -- URL PARAMETER $vector->{vuln}->{key}: tags can be injected outside of delimeters");
		}
	    },
	    "$delims[1]\"" => sub {
		my ($state, $vector) = @_;
		if($state->{in_double}) {
		    $vector->src_tree($self->{mod_tag});
		    $vector->has_vuln({
			key => $key,
			double => 1,
				      });
		    $self->api("CORE")->log_vuln($vector, "Discovered cross-site scripting (XSS) vulnerability: $vector->{data}->{method} $vector->{data}->{path} -- URL PARAMETER $vector->{vuln}->{key}: double quotes can be injected to escape delimeters in HTML");
		}
	    },
	    "$delims[2]'" => sub {
		my ($state, $vector) = @_;
		if($state->{in_quote} && $state->{in_script}) {
		    $vector->src_tree($self->{mod_tag});
		    $vector->has_vuln({
			key => $key,
			double => 1,
				      });
		    $self->api("CORE")->log_vuln($vector, "Discovered cross-site scripting (XSS) vulnerability: $vector->{data}->{method} $vector->{data}->{path} -- URL PARAMETER $vector->{vuln}->{key}: single quotes can be injected to escape delimeters in javascript code");
		}
	    },
	    );

	my %state = (
	    in_tags => 0,
	    in_quotes => 0,
	    in_double => 0,
	    in_script => 0,
	    );

	while($test_content =~ /($delims[0]$tag(?=$delims[1]|$delims[2]|$delims[3])|$delims[1]"(?=$delims[2]|$delims[0]|$delims[3])|$delims[2]'(?=$delims[3]|$delims[1]|$delims[0])|(?!<\\)'|(?!<\\)"|<script.*?>|<\/script>|<|>)/g) {
	    my $match = $1;

	    if($match =~ /<script/) {
		if($match =~ /src/) {
		    next;
		} else {
		    $match = "<script>";
		}
	    }

	    $states{$match}->(\%state, $test_vector);
	}
    }
}

sub generate_string
{
    my $max_len = shift;

    my $alpha = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890';

    my $len = int(rand($max_len - 1)) + 1;

    my $str;
    for(1..$len) {
	my $i = int(rand(length($alpha)));
	$str .= substr($alpha, $i, 1);
    }

    return $str;
}

1;
