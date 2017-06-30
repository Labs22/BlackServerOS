###################### dumb0 #######################
# Coded by The X-C3LL (J.M. Fernández)             #
# Email: overloadblog////hotmail////es             #
# Blog: 0verl0ad.blogspot.com                      #
# Twitter: https://twitter.com/TheXC3LL            #
######################  v0.1.2  ####################
#Dumb0: Simple Script to harvest usernames in populars forums and CMS
#    Copyright (C) 2014  Juan Manuel Fernández

 #   This program is free software: you can redistribute it and/or modify
  #  it under the terms of the GNU General Public License as published by
   # the Free Software Foundation, either version 3 of the License, or
    #(at your option) any later version.

    #This program is distributed in the hope that it will be useful,
    #but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 #   GNU General Public License for more details.

  #  You should have received a copy of the GNU General Public License
   # along with this program.  If not, see <http://www.gnu.org/licenses/>.
   
   
   
   
use LWP::UserAgent;
use Getopt::Long;

GetOptions(
		"type=s"=> \$flag_type,
		"url=s"=> \$flag_url,
		"log"=>  \$flag_log,
		"file=s"=> \$flag_file
         );

print q(
    ;'*¨'`·- .,  ‘               .-,             ,'´¨';'        ,·'´¨;.  '                         ,.  - · - .,  '              , ·. ,.-·~·.,   ‘    
    \`:·-,. ,   '` ·.  '         ;  ';\          ,'   ';'\'      ;   ';:\           .·´¨';\   ,·'´,.-,   ,. -.,   `';,'          /  ·'´,.-·-.,   `,'‚    
     '\:/   ;\:'`:·,  '`·, '     ';   ;:'\        ,'   ,'::'\    ;     ';:'\      .'´     ;:'\   \::\.'´  ;'\::::;:'  ,·':\'       /  .'´\:::::::'\   '\ °  
      ;   ;'::\;::::';   ;\     ';  ';::';      ,'   ,'::::;    ;   ,  '·:;  .·´,.´';  ,'::;'    '\:';   ;:;:·'´,.·'´\::::';   ,·'  ,'::::\:;:-·-:';  ';\‚  
      ;  ,':::;  `·:;;  ,':'\'   ';  ';::;     ,'   ,'::::;'    ;   ;'`.    ¨,.·´::;'  ;:::;     ,.·'   ,.·:'´:::::::'\;·´   ;.   ';:::;´       ,'  ,':'\‚ 
     ;   ;:::;    ,·' ,·':::;   ';  ';::;    ,'   ,'::::;'     ;  ';::; \*´\:::::;  ,':::;‘     '·,   ,.`' ·- :;:;·'´        ';   ;::;       ,'´ .'´\::';‚
     ;  ;:::;'  ,.'´,·´:::::;    \   '·:_,'´.;   ;::::;‘    ';  ,'::;   \::\;:·';  ;:::; '        ;  ';:\:`*·,  '`·,  °    ';   ':;:   ,.·´,.·´::::\;'°
    ':,·:;::-·´,.·´\:::::;´'      \·,   ,.·´:';  ';:::';     ;  ';::;     '*´  ;',·':::;‘          ;  ;:;:'-·'´  ,.·':\       \·,   `*´,.·'´::::::;·´   
     \::;. -·´:::::;\;·´          \:\¯\:::::\`*´\::;  '   \´¨\::;          \¨\::::;        ,·',  ,. -~:*'´\:::::'\‘      \\:¯::\:::::::;:·´      
      \;'\::::::::;·´'               `'\::\;:·´'\:::'\'   '    '\::\;            \:\;·'          \:\`'´\:::::::::'\;:·'´        `\:::::\;::·'´  °       
         `\;::-·´                               `*´°         '´¨               ¨'             '\;\:::\;: -~*´‘                ¯                  
                                                 '                                                     '                        ‘   
								http://0verl0ad.blogspot.com               

);


if (!$flag_type or !$flag_url) {
	&use;
	exit;
}

if ($flag_file) {
open(FILE,">>", $flag_file);
}

if ($flag_type eq "SMF") { $tail = "/index.php?action=profile;u="; }
if ($flag_type eq "IPB") { $tail = "/index.php?showuser="; }
if ($flag_type eq "XEN") { $tail = "/members/"; }
if ($flag_type eq "VB") { $tail = "/member.php?u="; }
if ($flag_type eq "myBB") { $tail = "/user-"; $add = ".html";}
if ($flag_type eq "useBB") { $tail = "/profile.php?id="; }
if ($flag_type eq "vanilla") { $tail = "/account/"; }
if ($flag_type eq "bbPress") { $tail = "/profile.php?id="; }
if ($flag_type eq "WP") { $tail = "/?author="; }
if ($flag_type eq "SPIP") { $tail = "/spip.php?auteur"; }
if ($flag_type eq "MOODLE") { $tail = "/user/view.php?id="; }
if ($flag_type eq "DRUPAL") { &drupal($flag_url); } 
if ($flag_type eq "BEE") { $tail = "/user_profile.php?uid="; }
if ($flag_type eq "FLUX") { $tail= "/profile.php?id="; }
if ($flag_type eq "FUD") { $tail = "/index.php?t=usrinfo&id=";}
if ($flag_type eq "punBB") { $tail = "/profile.php?id="; }
if ($flag_type eq "ACM") { $tail = "/?page=profile&id="; }
if ($flag_type eq "BURN") { $tail= "/profile.php?userid="; }
if ($flag_type eq "COM") { $tail = "/user/Profile.aspx?UserID="; }
if ($flag_type eq "deluxeBB") { $tail = "/misc.php?sub=profile&uid="; }
if ($flag_type eq "fusionBB") { $tail = "/showuser.php?uid/"; }
if ($flag_type eq "JFORUM") { $tail = "/jforum/user/profile/"; $add = ".page"; }
if ($flag_type eq "JITBIT") { $tail = "/viewprofile.aspx?UserID="; }
if ($flag_type eq "JIVE") { $tail = "/profile/"; }
if ($flag_type eq "NEAR") { $tail = "/users/"; }
if ($flag_type eq "OVBB") { $tail = "/member.php?action=getprofile&userid="; }
if ($flag_type eq "TikiWiki") { $tail = "/tiki-user_information.php?userId="; }


if ($flag_log) {
	print "[!] Insert the session cookie\n\n";
	print "/!\\ Cookie> ";
	$cookie = <STDIN>;
	chomp($cookie);
}
$i = "1";


$ua = LWP::UserAgent->new; $ua->agent('Mozilla/5.0 (X11; Linux i686; rv:17.0) Gecko/20131030');
$response = $ua->get($flag_url.$tail."2", Cookie => $cookie);
if (!$response->is_success) { die "[-] ERROR: URL couldn't be reached (Wrong URL?)\n"; }

print "[!] You need to configure the patterns\n";
print "[!] Copy and paste what is BEFORE and AFTER the user nick\n\n";


$html = $response->decoded_content;
@contenido = split("\n", $html);
foreach $linea (@contenido) {
	if ($linea =~ m/\<title\>(.*?)\<\/title\>/g) {
		print "/!\\-> ".$1."\n\nAFTER > ";
		$patron = <STDIN>;
		chomp($patron);
		$size = length($patron);
		print "\n\nBEFORE > ";
		$begin = <STDIN>;
		chomp($begin);
		$bsize = length($begin);
	}
}
 

print "\n[!] Dumping users from $flag_url...\n\n";

$j = 0;
while ($j <= 10) { 
		$ua = LWP::UserAgent->new; $ua->agent('Mozilla/5.0 (X11; Linux i686; rv:17.0) Gecko/20131030');
		$response = $ua->get($flag_url.$tail.$i.$add, Cookie => $cookie);
		$html = $response->decoded_content; 
		@contenido = split("\n", $html);
		foreach $linea (@contenido) {
			if ($linea =~ m/\<title\>(.*?)\<\/title\>/g) {
				$titulo = $1; 
			}

		}
		if ($response->status_line =~ /404/) {
			$j++; $i++;
		} else { 
			$tl = length($titulo);
			$ul = $tl - $size;
			$usuario = substr($titulo,0, $ul);
			$ul = length($usuario);
			$usuario = substr($usuario, $bsize, $ul);
			if ($flag_file) { print FILE $usuario."\n";}
			print "[+] Posible user found ~> ".$usuario."\n";
			$i++; $j = 0;
		}
	}	


print "[!] Work finished\n\n";

sub use {
print q(
	Use: perl dumb0.pl --type=[CMS] --url=[TARGET URL] [--log] [--file]
		
	Supported: 
			SMF   	 --		Simple Machine Forums
			IPB   	 --		Invision Power Board
			XEN      --		Xen Foro
			VB       --		vBulletin
			myBB     --
			useBB    --
			vanilla  --
			bbPress  --
			WP	 --		WordPress
			SPIP	 --		SPIP CMS
			DRUPAL   --		Drupal
			MOODLE 	 --		Moodle 
			BEE	 --		Beehive Forums
			FLUX	 --		fluxBB
			FUD	 --		FUDforum
			punBB	 --
			ACM	 --		AcmImBoard XD
			BURN	 --		Burning Board
			COM	 --		Community Servers
			deluxeBB --		
			fusionBB --
			JFORUM	 --
			JITBIT 	 --		Jibit ASPNetForum
			JIVE	 --		Jive Forums
			NEAR	 --		Near Forums
			OVBB	 --		
			TikiWiki --		TikiWiki CMS-Groupware
			
);
}


sub drupal {
	$target = $_[0];
	$ua = LWP::UserAgent->new; $ua->agent('Mozilla/5.0 (X11; Linux i686; rv:17.0) Gecko/20131030');
	for ($i = 33; $i++; $i < 127) {
		$url = $target."/?q=admin/views/ajax/autocomplete/user/".chr($i);
		print "[+] Checking for users wich nicks start with... ".chr($i)."\n";
		$response = $ua->get($url);
		$string = $response->decoded_content;
		chop($string);
		$string = substr($string,1);
		@usuarios = split(",", $string);
		foreach $user (@usuarios) {
			@nick = split('":"', $user);
			$user_clean = substr($nick[0], 1);
			$user_clean =~ s/\\u0027/\'/g;
			if ($flag_file) { print FILE $user_clean."\n";}
			print "\t\t[-] User found: $user_clean\n";
		}
	}
}


close(FILE);
