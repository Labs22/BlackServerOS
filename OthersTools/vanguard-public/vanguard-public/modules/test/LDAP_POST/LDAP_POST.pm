package Vanguard::LDAP_POST;
use base Vanguard::Module;
use strict;
use Vanguard::Vector;

sub new
{
    my $class = shift;
    my $self = {
	scan => shift,
	mod_tag => 'LDAP_POST',
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

    my @ldap_true = @{$self->{config}->{ldap_true}};
    my @ldap_false = @{$self->{config}->{ldap_false}};

    my $i = 0;
    while($i < 2) {
	foreach my $key (keys %{$vector->{data}->{post_params}}) {
	    my $true_vector = $vector->clone();
	    $true_vector->{data}->{post_params}->{$key}->{value} .= $ldap_true[$i];
	    
	    my $false_vector = $vector->clone();
	    $false_vector->{data}->{post_params}->{$key}->{value} .= $ldap_false[$i];
	    
	    my $content = $vector->{data}->{response}->{whisker}->{data};
	    
	    my $true_content = $self->api("WEBAPPS")->do_request($true_vector);
	    my $false_content = $self->api("WEBAPPS")->do_request($false_vector);
	    
	    if(($true_content ne $false_content) &&
	       (($true_content eq $content) || ($false_content eq $content))) {
		$vector->src_tree($self->{mod_tag});
		$vector->has_vuln({
		    key => $key,
		    # md5 of content?
				  });
		$self->api("CORE")->log_vuln($vector, "Discovered LDAP injection vulnerability: $vector->{data}->{method} $vector->{data}->{path} -- POST DATA $vector->{vuln}->{key}");
	    }
	}
	$i++;
    }
}

1;
