package Vanguard::CRAWL;
use base Vanguard::Module;
use strict;
use LW2;
use Vanguard::Vector;

my $gself;

sub new
{
    my $class = shift;
    my $self = {
	scan => shift,
	mod_tag => 'CRAWL',
	fingerprints => undef,
    };
    bless $self, $class;

    return $self;
}

sub act
{
    my $self = shift;
    my $vector = shift;

    $self->{host} = $vector->{data}->{host};

    $self->crawl($vector);
}

sub is_target
{
    my $self = shift;
    my $vector = shift;

    if($vector->src_tree() eq 'NMAP' && $vector->{data}->{protocol} =~ /http|ssl/ig
       && $vector->{data}->{state} eq 'open') {
	return 1;
    }
    return 0;
}

sub crawl
{
    my $self = shift;
    my $vector = shift;

    my $protocol = $vector->{data}->{protocol};

    $protocol = 'HTTP' if $vector->{data}->{protocol} =~ /http$/i;
    $protocol = 'HTTPS' if $vector->{data}->{protocol} =~ /https|ssl/i;

    # Create a request to crawl with
    my $request = LW2::http_new_request(
        host => $vector->{data}->{host},
        method => 'GET',
        timeout => 10,
        port => $vector->{data}->{port},
        encode_anti_ids => $self->{scan}->{config}->{evasion},
	protocol => $protocol,
        );

    # Can this be done in http_new_request?
    $request->{'User-Agent'} = $self->{scan}->{config}->{user_agent};

    # Make sure everything is compliant
    LW2::http_fixup_request($request);

    my $crawler = LW2::crawl_new(
        $protocol . "://" . $vector->{data}->{host} . '/',
        $self->{config}->{depth},
        $request
        );

    $crawler->{config}->{save_cookies}    = 1;
    $crawler->{config}->{save_skipped}    = 1;
    $crawler->{config}->{use_params}      = 1;
    $crawler->{config}->{source_callback} = \&callback;
    $crawler->{config}->{callback}        = 0;
    $crawler->{config}->{follow_moves}    = 1;
    $crawler->{config}->{normalize_uri}   = 1;

    $gself = $self; # gross
    $self->{scan}->vgprint("$self->{mod_tag}: Crawling $vector->{data}->{host}:$vector->{data}->{port}");
    my $result = $crawler->{crawl}->();

    if(not defined $result) {
        print STDERR "Error while crawling!\n";
        print STDERR "$crawler->{errors}->[0]\n\n";
    }
}

sub get_type
{
    my $value = shift;

    return 'str' if ($value =~ /^[A-Z^\d]+$/gi);
    return 'int' if ($value =~ /^[\d]+$/g);
    return undef; # unknown/other
}

sub get_rewrite_type
{
    my $value = shift;

    if($value =~ /^\d+$/) {
	return 'int';
    } elsif($value =~ /^[a-zA-Z]+\d*$/) {
	return 'str';
    } elsif($value =~ /^[a-zA-Z]+$/) {
	return $value;
    } elsif($value =~ /^[a-zA-Z0-9]+$/) {
	return 'alphanum';
    } else {
	return 'other';
    }
}

sub callback
{
    my ($crawl_obj_ref) = @_;
    my %crawl_obj = %{$crawl_obj_ref};

    my $request = $crawl_obj{request};
    my $response = $crawl_obj{response};

    my $vector = new Vanguard::Vector;

    $vector->src_tree($gself->{mod_tag});
    $vector->data({
	request => Clone::clone($request),
	response => Clone::clone($response),
	method => 'GET',
	host => $gself->{host},
		  });

    $gself->api("WEBAPPS")->store_cookies($response);

    $request->{whisker}->{uri} =~ /(.*)\?/;
    my $path = $1;
    if($path) {
	$vector->{data}->{path} = $path;
    } else {
	$vector->{data}->{path} = $request->{whisker}->{uri};
    }

    # Rewrite
    if($request->{whisker}->{uri} !~ /\?/ && # No URL params...
       $request->{whisker}->{uri} =~ /\/.*\//) { # and at least two slashes
	if($gself->{scan}->{config}->{rewrite}) {
	   $vector->{path} = '/';

	   my $fp;
	   my $i = 0;
	   while($request->{whisker}->{uri} =~ /(?<=\/)(.*)(?=\/)/g) {
	       my $value = $1;
	       my $variable = $i;
	       
	       $vector->{data}->{get_params}->{$variable}->{type} = get_type($value);
	       $vector->{data}->{get_params}->{$variable}->{value} = $value;

	       $fp .= get_rewrite_type($value);
	   }
	   $vector->{data}->{rewrite} = 1;
	   $vector->{data}->{rewrite_fp} = $fp;
	}
    } else {
	while($request->{whisker}->{uri} =~ /[&\?\;]([^=]+)=((?!&)[^&\?\;]*)/g) {
	    my $variable = $1;
	    my $value = $2;
	    
	    $vector->{data}->{get_params}->{$variable}->{type} = get_type($value);
	    $vector->{data}->{get_params}->{$variable}->{value} = $value;
	}
    }

    if(not $gself->redundant($vector)) {
	$gself->{scan}->queue_vector($vector);
	$gself->parse_forms($vector);
    }
}

sub parse_forms
{
    my $self = shift;
    my $vector = shift;

    my $html = $vector->{data}->{response}->{whisker}->{data};

    my $found = LW2::forms_read(\$html);

    # for each form...
    foreach my $form (@{$found}) {
        my $path = $form->{"\0"}->[2];

        $path =~ s/&amp;/&/g;

        my $use_ssl;

	if(($vector->{data}->{request}->{whisker}->{ssl} && $path !~ /^http\:/i) ||
	   $path =~ /^https/i) {
	    $use_ssl = 1;
	}

	if($path =~ /^\.(.*)/) { # if its a reletive path make it absolute
            my $newpath = $1; # a good place for a dealer to hide...
            $vector->{data}->{request}->{whisker}->{uri} =~ /(.*)\//;
            my $newuri = $1;
            $path = $newuri . $newpath;
        }

        # Form name is not important
	my $method = uc $form->{"\0"}->[1]; # GET/POST  

	my $vector = new Vanguard::Vector;
	$vector->src_tree($self->{mod_tag});

	$vector->{data}->{method} = $method;
	$vector->{data}->{path} = $path;

	my $protocol = 'HTTP';
	$protocol = 'HTTPS' if $use_ssl;

	my $request = LW2::http_new_request(
	    host => $vector->{data}->{host},
	    method => $method,
	    timeout => 10,
	    port => $vector->{data}->{port},
	    encode_anti_ids => $self->{scan}->{config}->{evasion},
	    protocol => $protocol,
	    ssl => $use_ssl,
	    uri => $path,
	    );

	# libwhisker does this already...
        $vector->{data}->{referrer} = $protocol . '://' . $vector->{data}->{host} . $vector->{data}->{request}->{whisker}->{uri};

        # Get POST args
        foreach my $key (keys %{$form}) {
            if($key =~ m/^unknown[0-9]+$/) {
                # Ignore nameless options for now
                next;
            }

            my $type = $form->{$key}->[0]; # Don't know that the lc is necessary
            if(ref($type) eq 'ARRAY') { # If its an array
                $type = $type->[0];
            } else {
                next; # I dunno
            }

	    $vector->{data}->{post_params}->{$key}->{type} = $type;

            my $value;
            my $count = 0;
            foreach(@{$form->{$key}}) {
                # Get the second possible value if there is one, to avoid pesky things like
                # "Choose one"
                # TODO: Add option to disable this	     
		$value = $_->[1];
                if($count > 0) {
                    $value = $_->[1];
                    last;
                }
                $count++;
            }

	    chomp $value;
	    $vector->{data}->{post_params}->{$key}->{value} = $value;
	}

	my $data;
	foreach my $key (keys %{$vector->{data}->{post_params}}) {
	    $data .= "$key=$vector->{data}->{post_params}->{$key}->{value}&";
	}
	chop $data; # remove trailing &

	if($method eq 'GET') {
	    $request->{whisker}->{uri} = "?$data";
	} elsif($method eq 'POST') {
	    $request->{whisker}->{data} = $data;
	}

	LW2::http_fixup_request($request);

	$vector->{data}->{request} = $request;

	if(not $self->redundant($vector)) {
	    my %response;

	    if(not LW2::http_do_request($request, \%response)) {
		$vector->{data}->{response} = \%response;
	    } else {
		# Maybe ignore this form?
	    }

	    $self->{scan}->queue_vector($vector);
	}
    }
}

sub redundant
{
    my $self = shift;
    my $vector = shift;
    
    if($vector->{data}->{rewrite_fp}) {
	return $self->{scan}->is_redundant($vector->{data}->{rewrite_fp});
    } else {
	my $fingerprint = $vector->{data}->{type} . $vector->{data}->{path};
	
	foreach my $key (keys %{$vector->{data}->{get_params}}) {
	    $fingerprint .= $key;
	}
	
	foreach my $key (keys %{$vector->{data}->{post_params}}) {
	    $fingerprint .= $key;
	}
	
	return $self->{scan}->is_redundant($fingerprint);
    }
}

1;
