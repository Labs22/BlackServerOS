package Vanguard::SQL_POST;
use base Vanguard::Module;
use strict;
use LW2;
use Digest::MD5 qw(md5_hex);
use Vanguard::Vector;

sub new
{
    my $class = shift;
    my $self = {
	scan => shift,
	mod_tag => 'SQL_POST',
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

    my @sql_entries = @{$self->{config}->{sql_entries}};
    my @sql_exits   = @{$self->{config}->{sql_exits}};
    my @sql_spacers = @{$self->{config}->{sql_spacers}};

    my $int = int(rand(100));
    my $rnt = int(rand(100));

    until ($int ne $rnt || $int != $rnt)
    {
        $int = int(rand(100));
    }

    my $sql_false = "$int=$rnt";
    my $sql_true  = "$rnt=$rnt";

    my $content = $vector->{data}->{response}->{whisker}->{data};

    foreach my $key (keys %{$vector->{data}->{post_params}}) {
	foreach my $sql_entry (@sql_entries) {
	    foreach my $sql_spacer (@sql_spacers) {
		foreach my $sql_exit (@sql_exits) {
		    my $test_vector = $vector->clone();
		    my $verify_vector = $vector->clone();
		    
		    $test_vector->{data}->{post_params}->{$key}->{value} .= $sql_entry . $sql_spacer . 'OR' . $sql_spacer . $sql_true . $sql_exit;
		    $verify_vector->{data}->{post_params}->{$key}->{value} .= $sql_entry . $sql_spacer . 'OR' . $sql_spacer . $sql_false . $sql_exit;
		    
		    my $test_content = $self->api("WEBAPPS")->do_request($test_vector);
		    my $verify_content = $self->api("WEBAPPS")->do_request($verify_vector);
		    
		    if(($content eq $test_content &&
			$test_content ne $verify_content) ||
		       ($content eq $verify_content &&
			$verify_content ne $test_content)) {
			$vector->src_tree($self->{mod_tag});
			$vector->has_vuln({
			    key => $key,
			    sql_spacer => $sql_spacer,
			    sql_cmp => 'OR',
			    sql_true => $sql_true,
			    sql_false => $sql_false,
			    sql_entry => $sql_entry,
			    sql_exit => $sql_exit,
			    true_md5 => md5_hex($test_content),
			    false_md5 => md5_hex($verify_content),
					  });
			$self->api("CORE")->log_vuln($vector, "Discovered SQL injection vulnerability: $vector->{data}->{method} $vector->{data}->{path} -- POST DATA $vector->{vuln}->{key}: $sql_entry$sql_spacer" . 'OR' . "$sql_spacer$sql_true$sql_exit");
		    }
		}
	    }
	}
    }
}

1;

