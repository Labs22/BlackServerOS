#!/usr/bin/tclsh

#
# Generates binary packets specified using an APD-like data format:
# http://wiki.hping.org/26
#
# For every additions please send me an email (eventually with your modifies)
#

#
# The output (STDOUT) is binary
#
fconfigure stdout -translation binary

#
# Size conversion table for "binary format"
#
dict set size_to_format "8bits"  "c"
dict set size_to_format "16bits" "S"
dict set size_to_format "32bits" "I"
dict set size_to_format "hexstr" "H*"

#
# Supported protocols
#
set supported_protocols {ethernet arp ip icmp udp tcp tcp.end tcp.nop tcp.timestamp data}

# 
# Convert the value in a binary protocol field
# 
proc binary_field  {value size translation_table} {
    global size_to_format

    # first let's see if we can translate the value
    # using the table (for mnemonic words...)
    if {[dict exists $translation_table $value]} {
        set value [dict get $translation_table $value]
    }

    # binary conversion
    return [binary format [dict get $size_to_format $size] $value]
}

#
# Search the table for "name"
# il present return its value, otherwise return "def_val"
#
proc get_val {name table def_val} {
    if {[dict exists $table $name]} {
        return [dict get $table $name]
    } else {
        return $def_val
    }
}

#
# Utilities to convert addresses
#

proc mac_to_hexstr {addr} {
    regsub -all {[^0-9a-fA-F]} $addr {} mac

    if [expr ([string length $mac] != 12)] {
        puts stderr "error: mac address provided is invalid..."
        exit 1
    }

    return $mac
}

proc ip_to_num {addr} {
    if [string match "*.*.*.*" $addr] {
        set bytes [split $addr "."]
        return [expr ([lindex $bytes 0] * (2 ** 24)) + \
                     ([lindex $bytes 1] * (2 ** 16)) + \
                     ([lindex $bytes 2] * (2 ** 8))  + \
                     ([lindex $bytes 3])]
    } else {
        return $addr
    }
}


#
# Ethernet
#
proc ethernet {fields} {

    # mnemonic substitutions
    dict set translation_table ip  0x0800
    dict set translation_table arp 0x0806

    # prepare mac addresses
    set dst [mac_to_hexstr [get_val "dst"  $fields "FFFFFFFFFFFF"]]
    set src [mac_to_hexstr [get_val "src"  $fields "FFFFFFFFFFFF"]]

    # print fields
    foreach item [list                                   \
        [list $dst "hexstr"]                             \
        [list $src "hexstr"]                             \
        [list [get_val "type" $fields 0x0800] "16bits"]  \
    ] {
        puts -nonewline [binary_field  [lindex $item 0] \
                                       [lindex $item 1] \
                                       $translation_table]
    }

}

#
# ARP
#
proc arp {fields} {

    # mnemonic substitutions
    dict set translation_table ethernet        0x0001
    dict set translation_table ip              0x0800
    dict set translation_table request         0x0001
    dict set translation_table reply           0x0002
    dict set translation_table request_reverse 0x0003
    dict set translation_table reply_reverse   0x0004

    # prepare mac addresses
    set shard [mac_to_hexstr [get_val "shard"  $fields "FFFFFFFFFFFF"]]
    set thard [mac_to_hexstr [get_val "thard"  $fields "FFFFFFFFFFFF"]]

    # prepare ip addresses
    set sproto [ip_to_num [get_val "sproto" $fields 192.168.0.5]]
    set tproto [ip_to_num [get_val "tproto" $fields 192.168.0.1]]   

    # print fields
    foreach item [list                                     \
        [list [get_val "htype"  $fields 0x0001] "16bits"]  \
        [list [get_val "ptype"  $fields 0x0800] "16bits"]  \
        [list [get_val "hsize"  $fields 0x06]   "8bits"]   \
        [list [get_val "psize"  $fields 0x04]   "8bits"]   \
        [list [get_val "op"     $fields 0x0000] "16bits"]  \
        [list $shard                            "hexstr"]  \
        [list $sproto                           "32bits"]  \
        [list $thard                            "hexstr"]  \
        [list $tproto                           "32bits"]  \

    ] {
        puts -nonewline [binary_field  [lindex $item 0] \
                                       [lindex $item 1] \
                                       $translation_table]
    }

}

#
# IPv4
#
proc ip {fields} {

    # mnemonic substitutions
    dict set translation_table icmp 0x01
    dict set translation_table tcp  0x06
    dict set translation_table udp  0x11
    
    # prepare ihl + ver
    set ihl [get_val "ihl" $fields 5]
    set ver [get_val "ver" $fields 4]
    set ihl_ver [expr ($ver << 4) + $ihl]

    # prepare flags + fragoff
    set flags [get_val "flags" $fields 2]
    set fragoff [get_val "fragoff" $fields 0]
    set flags_fragoff [expr ($flags << 13) + $fragoff]

    # prepare ip addresses
    set saddr [ip_to_num [get_val "saddr" $fields 192.168.0.5]]
    set daddr [ip_to_num [get_val "daddr" $fields 192.168.0.1]]

    # print fields
    foreach item [list                                      \
        [list $ihl_ver                        "8bits"]      \
        [list [get_val "tos"    $fields 0xc0] "8bits"]      \
        [list [get_val "totlen" $fields 0x00] "16bits"]     \
        [list [get_val "id"     $fields 0x01] "16bits"]     \
        [list $flags_fragoff                  "16bits"]     \
        [list [get_val "ttl"    $fields 0xc0] "8bits"]      \
        [list [get_val "proto"  $fields 0x00] "8bits"]      \
        [list [get_val "cksum"  $fields 0x00] "16bits"]     \
        [list $saddr                          "32bits"]     \
        [list $daddr                          "32bits"]     \
    ] {
        puts -nonewline [binary_field  [lindex $item 0]     \
                                       [lindex $item 1]     \
                                       $translation_table]
    }

}

#
# ICMP
#

proc icmp {fields} {

    # mnemonic substitutions
    dict set translation_table reply 0x00
    dict set translation_table echo  0x08

    # print fields
    foreach item [list                                  \
        [list [get_val "type"  $fields 0x08] "8bits"]   \
        [list [get_val "code"  $fields 0x00] "8bits"]   \
        [list [get_val "cksum" $fields 0x00] "16bits"]  \
        [list [get_val "id"    $fields 0x01] "16bits"]  \
        [list [get_val "seq"   $fields 0x01] "16bits"]  \
    ] {
        puts -nonewline [binary_field  [lindex $item 0] \
                                       [lindex $item 1] \
                                       $translation_table]
    }

}

#
# UDP
#

proc udp {fields} {

    # mnemonic substitutions
    set translation_table {}

    # print fields
    foreach item [list                                  \
        [list [get_val "sport" $fields 4444] "16bits"]  \
        [list [get_val "dport" $fields 4445] "16bits"]  \
        [list [get_val "len"   $fields 0x00] "16bits"]  \
        [list [get_val "cksum" $fields 0x00] "16bits"]  \
    ] {
        puts -nonewline [binary_field  [lindex $item 0] \
                                       [lindex $item 1] \
                                       $translation_table]
    }

}

#
# TCP
#

proc prepare_tcp_flags {mnemonic} { 

    set flags 0

    # mnemonic substitutions
    dict set translation_table u 0x20
    dict set translation_table a 0x10
    dict set translation_table p 0x08
    dict set translation_table r 0x04
    dict set translation_table s 0x02
    dict set translation_table f 0x01

    dict for {flag value} $translation_table {
        if [expr [string first $flag [string tolower $mnemonic]] != -1] {
            set flags [expr $flags + $value]
        }
    }

    return $flags

}

proc tcp {fields} {

    # mnemonic substitutions
    set translation_table {}
   
    # prepare ihl + ver
    set off    [get_val "off" $fields 5]
    set ns     [get_val "ns"  $fields 0]
    set off_ns [expr ($off << 4) + $ns]

    # prepare flags + cwr + ece
    set flags [get_val "flags" $fields 0x02]
    if [string is alpha $flags] {
        set flags [prepare_tcp_flags $flags]
    }
    set cwr [get_val "cwr" $fields 0]
    set ece [get_val "ece" $fields 0]
    set cwr_ece_flags [expr ($cwr << 7) + ($ece << 6) + $flags]

    # print fields
    foreach item [list                                  \
        [list [get_val "sport" $fields 4444] "16bits"]  \
        [list [get_val "dport" $fields 4445] "16bits"]  \
        [list [get_val "seq"   $fields 0x01] "32bits"]  \
        [list [get_val "ack"   $fields 0x00] "32bits"]  \
	[list $off_ns                        "8bits"]  \
	[list $cwr_ece_flags                 "8bits"]  \
        [list [get_val "win"   $fields 62694] "16bits"]  \
        [list [get_val "cksum" $fields 0x00] "16bits"]  \
        [list [get_val "urp"   $fields 0x00] "16bits"]  \
    ] {
        puts -nonewline [binary_field  [lindex $item 0] \
                                       [lindex $item 1] \
                                       $translation_table]
    }

}

proc tcp.end {fields} {
     
    # mnemonic substitutions
    set translation_table {}

    # print fields
    foreach item [list                                \
        [list [get_val "kind" $fields 0x00] "8bits"]  \
    ] {
        puts -nonewline [binary_field  [lindex $item 0] \
                                       [lindex $item 1] \
                                       $translation_table]
    }

}

proc tcp.nop {fields} {
     
    # mnemonic substitutions
    set translation_table {}

    # print fields
    foreach item [list                                \
        [list [get_val "kind" $fields 0x01] "8bits"]  \
    ] {
        puts -nonewline [binary_field  [lindex $item 0] \
                                       [lindex $item 1] \
                                       $translation_table]
    }

}

proc tcp.timestamp {fields} {
    
    # mnemonic substitutions
    set translation_table {}

    # print fields
    foreach item [list                                \
        [list [get_val "kind" $fields 0x08] "8bits"]  \
        [list [get_val "len"  $fields 0x0a] "8bits"]  \
        [list [get_val "val"  $fields 0x00] "32bits"]  \
        [list [get_val "ecr"  $fields 0x00] "32bits"]  \
    ] {
        puts -nonewline [binary_field  [lindex $item 0] \
                                       [lindex $item 1] \
                                       $translation_table]
    }

}

#
# Raw data
#

proc data {fields} {
    puts -nonewline [binary_field [get_val "str" $fields 0] "hexstr" {}]
}

#
# MAIN
#

# examples

set test1_packet  "ethernet(dst=ff:ff:ff:ff:ee:ee,src=aa:aa:ee:ff:ff:ff,type=0x0800)+ip(ihl=5,ver=4,tos=0xc0,totlen=58,id=62912,fragoff=0,mf=0,df=0,rf=0,ttl=64,proto=1,cksum=0xe500,saddr=192.168.1.7,daddr=192.168.1.6)+icmp(type=3,code=3,unused=0)+data(str=aaaa)+udp(sport=33169,dport=10,len=10,cksum=0x94d6)+data(str=aaaa)+arp(htype=ethernet,ptype=ip,hsize=6,psize=4,op=request,shard=00:11:22:33:44:55,sproto=192.168.1.1,thard=22:22:22:22:22:22,tproto=10.0.0.1)"

set test2_packet "ethernet(dst=ff:ff:ff:ff:ff:ff,src=ff:ff:ff:ff:ff:ff,type=0x0800)+ip(ihl=5,ver=4,tos=00,totlen=30,id=60976,fragoff=0,mf=0,df=1,rf=0,ttl=64,proto=tcp,cksum=0x40c9,saddr=192.168.1.9,daddr=173.194.44.95)+tcp(sport=32857,dport=80,seq=1804471615,ack=0,ns=0,off=5,flags=s,win=62694,cksum=0xda46,urp=0)"

set test3_packet "ethernet(dst=ff:ff:ff:ff:ff:ff,src=ff:ff:ff:ff:ff:ff,type=0x0800)+ip(ihl=5,ver=4,tos=00,totlen=30,id=60976,fragoff=0,mf=0,df=1,rf=0,ttl=64,proto=tcp,cksum=0x40c9,saddr=192.168.1.9,daddr=173.194.44.95)+tcp(sport=32857,dport=80,seq=1804471615,ack=0,ns=0,off=8,flags=s,win=62694,cksum=0xda46,urp=0)+tcp.nop()+tcp.nop()+tcp.timestamp(val=54111314,ecr=1049055856)+data(str=f0a)"

proc main {packet} {
    global supported_protocols

    # split input packet in layers
    # (thanks antirez! http://wiki.hping.org/47)
    foreach layer [split $packet +] {
        
        set t [split $layer ()]
        set name [lindex $t 0]
        set fields [lindex $t 1]
        
        # save fields
        foreach field [split $fields ,] {
            set t [split $field =]
            dict set field_dict [lindex $t 0] [lindex $t 1]
        }

	# in case of empty field dictionary
	dict set field_dict "dummy" "dummy"

        # generate layer
        if [expr [lsearch -exact $supported_protocols $name] != -1] {
            $name $field_dict
        } else {
            puts stderr "error: protocol '$name' not supported."
            exit 1
        }

        # cleanup
        unset field_dict
    }

}

if [expr [string equal "" [lindex $argv 0]] || [string equal "-h" [lindex $argv 0]]] {
    puts "Packets.tcl -- Generates binary packets specified using an"
    puts "               APD-like data format: http://wiki.hping.org/26"
    puts ""
    puts "usage:\n\tpackets.tcl 'APD packet description'"
    puts ""
    puts "example packets:"
    puts ""
    puts "$test1_packet\n"
    puts "$test2_packet\n"
    puts "$test3_packet"

} else {
    main [lindex $argv 0]
}

