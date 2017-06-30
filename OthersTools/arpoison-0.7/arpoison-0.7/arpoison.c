/*
 *
 * Arpoison -- The UNIX Arp Cache Update Utility
 *
 * $Id: arpoison.c,v 1.2 2014/02/07 01:17:03 buer Exp $
 *
 */


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <libnet.h>

#define ARP_REPLY 2
#define ARP_REQUEST 1

void usage()
{
	printf("Usage: -i <device> -d <dest IP> -s <src IP> -t <target MAC> -r <src MAC> "
		"[-a] [-w time between packets] [-n number to send]\n");
	exit(1);
}

int main(int argc, char *argv[])
{
        int n,c,z,o,op_code,wait;
	int packets_to_send = 0;
	int packets_sent = 0;
	int count = 0;
        u_long SrcIP, DstIP;
        char *device;
	char err_buf[LIBNET_ERRBUF_SIZE];
	unsigned int p[6];
	u_char DstHW[6]; 
        u_char SrcHW[6]; 

	/* added for libnet 1.1 */
	libnet_t *l;
	libnet_ptag_t eth_tag;
	libnet_ptag_t arp_tag;
	
	/* init */
	
	eth_tag = arp_tag = LIBNET_PTAG_INITIALIZER;
	wait = 1;
	op_code = ARP_REPLY;	
	device = "eth0";
	SrcIP = DstIP = 0;
	SrcHW[0] = DstHW[0] = 'N';

	while ((z = getopt(argc, argv, "ad:f:hi:r:n:s:t:w:")) != EOF) {
	
		switch (z) {
		
			case 'h':

				usage();
				break;
		
			case 'd':
		   		
				DstIP = inet_addr(optarg); 
		   		if (DstIP == -1) {	
	           	
					printf("Not a valid ip\n");
					exit(1);
		   		}
		   		break;

                	case 's':
                   		
				SrcIP = inet_addr(optarg);
                   		if (SrcIP == -1) {
		         		printf("Not a valid ip\n");
		         		exit(1);
		   		}
         	   		break;

          		case 'i':

		   		device = optarg;
		   		break;	
	    
			case 't':
		    		
				n = sscanf(optarg, "%x:%x:%x:%x:%x:%x",
                                     &p[0],
                                     &p[1],
				     &p[2],
				     &p[3],
				     &p[4],
				     &p[5]);		

                    		if (n != 6) {
			    
					printf("error parsing MAC\n");
			    		exit(1);
				}		     
		    
				for ( c = 0 ; c < 6 ; c++)
					DstHW[c] = p[c];
		       
                    		break; 

			case 'r':
		    	
				n = sscanf(optarg, "%x:%x:%x:%x:%x:%x",
                                     &p[0],
                                     &p[1],
				     &p[2],
				     &p[3],
				     &p[4],
				     &p[5]);		
                    
				if (n != 6) {
			    
					printf("error parsing MAC\n");
			     		exit(1);
				}		     
		    
				for ( c = 0 ; c < 6 ; c++)
					SrcHW[c] = p[c];

                    		break; 

			case 'w':
		    	
				wait = atoi(optarg);
		    		break;

			case 'n':
		    
				packets_to_send = atoi(optarg);  
		    		break;     
			
			case 'f':
				
		   		o++;
		    		break;
		
			case 'a':
		   
				op_code = ARP_REQUEST;
				break;
     
			default:
				usage();

		} // switch     

	} // while

	if(SrcIP == 0 || DstIP == 0 || SrcHW[0] == 'N' || DstHW[0] == 'N')
		usage();

	if (getuid()) {

		printf("Must be run as root\n");
		exit(1);
	}

	l = libnet_init(LIBNET_LINK, device, err_buf ); 

	if (l == NULL) {

		printf("libnet_init: error %s\n", err_buf);
		exit(1);
	}

	/* ARP header */

        arp_tag = libnet_build_arp( 
                1,              	/* hardware type */
                0x0800,         	/* proto type */
                6,              	/* hw addr size */
                4,              	/* proto addr size */ 
                op_code,             	/* ARP OPCODE */
                SrcHW,         		/* source HW addr */
                (u_char *)&SrcIP,       /* src proto addr */
                DstHW,        		/* dst HW addr */
                (u_char *)&DstIP,       /* dst IP addr */
                NULL,           	/* no payload */
                0,              	/* payload length */
        	l,			/* libnet tag */
		0);			/* ptag see man */
		 
	if (arp_tag == -1) {

		perror("libnet_build_arp");
		exit(1);
	} 

	/* ethernet header */
       
	eth_tag  = libnet_build_ethernet(
                DstHW,         /* dst HW addr */
                SrcHW,         /* src HW addr */
                0x0806,         /* ether packet type */
                NULL,           /* ptr to payload */
                0,              /* payload size */
                l,
		0);        /* ptr to packet memory */

	if (eth_tag == -1) {

		perror("libnet_build_ethernet");
		exit(1);
	}

	for (;;) {
	
		n = libnet_write(l);
           
		if (n == -1 )
			printf("libnet write error");

		count++;

		printf("ARP %s %d sent via %s\n", (op_code == ARP_REQUEST) ? "request" : "reply", count, device);

		if (packets_to_send) {

			packets_sent++;

			if (packets_sent >= packets_to_send)
				break;
		}

		sleep(wait);
	}

        libnet_destroy(l); 

	return 0;
}

//////////////////////////////////////////////////////////////////////
//	                                                            //
//           ///\\\            /*////\/\\/*/        /**********/    //
//          /*/  \*\           /*/         /*/      /*/       /*/   //
//         /*/    \*\          /*/         /*/      /*/       /*/   //
//        /*/      \*\	       /*/       /*/        /*/      /*/    //
//       /*//\/\/\/\\*\        /*/\//\\/*/          /********/	    //
//      /*/          \*\       /*/\\\\ 	            /*/		    //		
//     /*/	      \*\      /*/  \\\\            /*/		    //
//    /*/	       \*\     /*/     \\\\         /*/	            //
//   /*/	        \*\    /*/        \\\\      /*/		    //
//								    // 	
//////////////////////////////////////////////////////////////////////
