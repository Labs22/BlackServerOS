/*
  Tarefa 7 - MC942 - DNS Spoof Detect

  Anderson Toshiyuki Sasaki RA058908
  Davi Colli Tozoni RA060061
  Guilherme Henrique Goulart Pozzato RA061240
*/

#include <stdio.h>
#include <stdlib.h>
#include <pcap.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <string.h>

#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <resolv.h>

#define ETHER_ADDR_LEN  6

typedef unsigned short u16;
typedef unsigned long u32;

char * interface;

/* Estrutura para uma pergunta DNS */
struct question {
  char *qname;
  char qtype[2];
  char qclass[2];
};

/* Estrutura para uma resposta DNS */
struct answer {
  char *name;
  char atype[2];
  char aclass[2];
  char ttl[4];
  char RdataLen[2];
  char *Rdata;
};

/*cabecalho do DNS*/
struct dnshdr {
 char id[2];
 char flags[2];
 char qdcount[2];
 char ancount[2];
 char nscount[2];
 char arcount[2];
};

struct lista_req {
  char identificador[100];
  char pergunta[100];
  char IP[20][100];
  int nIP;
  struct lista_req * prox;
};

struct lista_req * no_cabeca;

/*função que retorna a posição da "pergunta" na lista ligada de requisições*/
struct lista_req *  procuraLista(char * id, struct lista_req ** anterior){
  struct lista_req * atual = no_cabeca;
  *anterior = NULL;

  /*procura por requisicao na lista*/
  while(atual!=NULL){
    if(!strcmp(atual->identificador, id)){
      break;
    }
    *anterior = atual;
    atual = atual->prox;
  }

  return atual;
}

/*função que remove elemento da lista*/
void removeLista(struct lista_req * elemento, struct lista_req * anterior){
  anterior->prox = elemento->prox;
  free(elemento);
}

/*função que adiciona IP ao elemento*/
void adicionaIPLista(struct lista_req * elemento, char * IP){
  int pos = elemento->nIP;
  strcpy(elemento->IP[pos], IP);
  (elemento->nIP)++;
}

/*função que insere novo elemento na lista*/
struct lista_req * adicionaLista(char * novo_id, char * nova_pergunta, char * IP){
  struct lista_req * novo = (struct lista_req *)malloc(sizeof(struct lista_req));
  strcpy(novo->identificador, novo_id);
  strcpy(novo->pergunta, nova_pergunta);
  
  novo->prox = no_cabeca->prox;
  no_cabeca->prox = novo;
  novo->nIP = 0;

  adicionaIPLista(novo, IP);

  return novo;
}

/* Faz o parsing de uma string contendo um endereço IP e o converte
 * num inteiro de 32 bits (4 bytes) */
unsigned int ipParser(char *ip){
  char string[16];
  char *aux;
  unsigned int ret = 0;

  strcpy (string, ip);

  aux = strtok(string, ".");
  ret += (atoi(aux) << 24);
  aux = strtok(NULL, ".");
  ret += (atoi(aux) << 16);
  aux = strtok(NULL, ".");
  ret += (atoi(aux) << 8);
  aux = strtok(NULL, ".");
  ret += atoi(aux);

  return ret;
}

/* estrutura para o header ethernet */
struct etherhdr{
  u_char ether_dhost[ETHER_ADDR_LEN]; /* endereço de destino */
  u_char ether_shost[ETHER_ADDR_LEN]; /* endereço de origem */
  u_short ether_type; /* protocolo da camada de rede */
};

/* Faz o parsing dos argumentos do programa */
void argParser(int argc, char *argv[]){
  int i;

  for(i=0; i<argc; i++){
    if(!strcmp(argv[i], "--interface")){
      interface = argv[i+1];
      i++;
    }
  }
}

/* Filtra e trata os pacotes de acordo com a expressão dada como parametro */
int packetFilter(char *expr){
  struct pcap_pkthdr header; 
  pcap_t *handle; 
  char errbuf[PCAP_ERRBUF_SIZE]; 
  struct bpf_program fp;
  const u_char *packet; 
  
  struct etherhdr *ether, *ether_novo;
  struct udphdr *udp, *udp_novo;
  struct iphdr *ip, *ip_novo;
  struct dnshdr *dns, *dns_novo;
  struct question query;
  char *query_nova;
  
  unsigned int size_ip_header = 0;

  char pergunta[200];
  char * atual;
  struct answer resposta;

  int i=0;
  int j=0;
  int k;
  int tam, tamanho_query, offset;
  int tamanho;

  bpf_u_int32 mask;    /* A máscara de rede da interface escutada */
  bpf_u_int32 net;     /* O ip da máquina escutada */


  if (pcap_lookupnet(interface, &net, &mask, errbuf) == -1) {
     fprintf(stderr, "ERRO: Não foi possível obter a máscara de rede (%s)\n", errbuf);
     net = 0;
     mask = 0;
  }

  /* Abre o arquivo de entrada para processamento */
  handle = pcap_open_live(interface, BUFSIZ, 1, 100000, errbuf); 
  if (handle == NULL) {
    printf("Erro ao ficar open! (%s)\n", errbuf);
    return(2);
  }

  /* Compila e aplica o filtro */
  if (pcap_compile(handle, &fp, expr, 0, 0) == -1) {
    fprintf(stderr, "Couldn't parse filter %s: %s\n", expr, pcap_geterr(handle));
    return(2);
  }
  if (pcap_setfilter(handle, &fp) == -1) {
    fprintf(stderr, "Couldn't install filter %s: %s\n", expr, pcap_geterr(handle));
    return(2);
  }

  /* captura e trata um pacote */
  while ((packet = pcap_next(handle,&header))) {

    /* acessa início de header ethernet */
    ether = (struct etherhdr*)(packet);

    /* acessa início de header ip */
    ip = (struct iphdr*)(((char*)packet) + 14);

    /* acessa início de header udp */
    size_ip_header = ip->ihl*4;
    udp = (struct udphdr *)(((char*) ip) + size_ip_header);
  
    /* acessa início de header dns */
    dns = (struct dnshdr*)(((char*) udp) + sizeof(struct udphdr));

    /* acessa querry do protocolo DNS */
    query.qname = (((char*)dns) + 12);
    atual = query.qname;
    tam = atual[0];

    /* monta nome da pergunta */
    j=0;
    i=0;
    while(tam!=0){
       i++; /* primeiro caracter */
       for(k=0; k<tam; k++){
               pergunta[j]=atual[i+k];
               j++;
       }

       pergunta[j]='.';
       j++;

       i+=tam; /* reposiciona indice */
       tam=atual[i];
    }

    j--; /* retira último ponto colocado */

    pergunta[j] = '\0';
    tamanho = j;
   
    int y=0;
    struct lista_req * atual_el = no_cabeca->prox;
    //printf("\n\nESTADO DA LISTA:\n");
    /*imprime lista atual*/
    /*while(atual_el!=NULL){
      printf("pergunta(id = %s): %s\n", atual_el->identificador, atual_el->pergunta);
      for(y=0; y<atual_el->nIP; y++){
        printf("IP(%d):%s\n", y, atual_el->IP[y]);
      }
      atual_el=atual_el->prox;
    }
    printf("\n\n");
    */

    /* Monta identificador da pergunta como uma string */
    char identificador[100];
    sprintf(identificador, "%hu", *((unsigned short *)dns->id));
    printf("A pergunta é: %s (id = %s)\n", pergunta, identificador); 
    
    /* Se se tratar de um DNS request, apaga elemento da lista com pergunta igual, se existir*/
    if(!(*((unsigned short*)(dns->ancount)))){
      printf(" e se trata de uma requisição.\n");
      struct lista_req ** anterior;
      anterior = (struct lista_req **)malloc(sizeof(struct lista_req *));
      struct lista_req * eliminado = procuraLista(identificador, anterior);
      if(eliminado!=NULL) removeLista(eliminado, *anterior); 
    }
    /* Se se tratar de um DNS reply, adiciona novo IP encontrado */
    else {
      printf(" e se trata de um reply.\n");
      struct lista_req ** anterior;
      anterior = (struct lista_req **)malloc(sizeof(struct lista_req *));
      struct lista_req * elemento;
      int q=0;

      elemento = procuraLista(identificador, anterior);

      int ja_existia = 0;

      char * comeco = (char *)query.qname + tamanho + 6;
      /*encontra os endereço IP contidos no reply*/
      printf("o número de respostas é %hu\n", htons(*((unsigned short int*)(dns->ancount))));
      for(q=0; q<htons(*((unsigned short int*)(dns->ancount))); q++){
        unsigned short int tipo = ((unsigned short int*)(comeco+2))[0];
        unsigned short int classe = ((unsigned short int*)(comeco+4))[0];
        unsigned short int resp_size = ((unsigned short int*)(comeco+10))[0];

        /*
         * Resposta:
         * bytes
         * ---- DNS HEADER ----
         * 0-1: ID
         * 2-3: Flags: QR Opcode(4) AA TC RD RA Z AD CD Rcode(4) -> 1 0000 0 0 0 0 0 0 0 0000
         * 4-5: # de perguntas -> 0
         * 6-7: # de RRs de respostas -> 1
         * 8-9: # de RRs com autoridade -> 0
         * 10-11: # de RRs adicionais -> 0
         * ---- payload ----
         * <size> + perguntas -> 0
         * <size> + respostas -> 1 
         *          TYPE(2B) + CLASS(2B) -> 1 + 1
         *          TTL
         *          RdataLen + Rdata:::
         * <size> + autoridade
         * <size> + info adicional
         *
         * */

        resposta.atype[0] = (char) 0;
        resposta.atype[1] = (char) 1;
        resposta.aclass[0] = (char) 0;
        resposta.aclass[1] = (char) 1;
        resposta.Rdata = "10.0.0.1";
        resposta.RdataLen = sizeof("10.0.0.1");
        res_send((char*)&query, tamanho, (char*)&resposta, 2 );

        printf("O tipo é %d\n", htons(tipo));
        printf("A classe é %d\n", htons(classe));
        printf("o resp_size é %d\n", htons(resp_size));
        if(htons(tipo) == 1){
          char IP[100];
          unsigned int IPi = ((unsigned int*)(comeco+12))[0];
          sprintf(IP, "%u.%u.%u.%u", ((unsigned char *)(&IPi))[0], ((unsigned char *)(&IPi))[1], ((unsigned char *)(&IPi))[2], ((unsigned char *)(&IPi))[3]);
          if(elemento !=NULL){
            adicionaIPLista(elemento, IP);
            ja_existia = 1;
          }
          else elemento = adicionaLista(identificador, pergunta, IP);

          //printf("end : %s;\n", IP);   
          //printf("size : %d;\n", htons(resp_size));
          comeco = comeco + 16;
        }
        else{
          comeco = comeco + 12 + htons(resp_size);
        }
      }

      /*Se já existia um reply de mesmo ID, imprime no STDOUT*/
      if(ja_existia){
        printf("Requisicao DNS sobre o dominio %s\n", elemento->pergunta);
        printf("Respostas: ");
        printf("%s", elemento->IP[0]);
        for(q=1; q<elemento->nIP; q++){
          printf(", %s", elemento->IP[q]);
        }
        printf("\n\n");
      }
      
    }
  }

  /* Fecha a sessão */
  pcap_close(handle);

  return 0;
}

/* Main do programa */
int main (int argc, char *argv[]){

  /* inicializa nó cabeça */
  no_cabeca = (struct lista_req *)malloc(sizeof(struct lista_req));
  no_cabeca->nIP = 0;
  no_cabeca->prox = NULL;

  argParser(argc, argv);

  char expr[1000];

  /* Cria a expressão utilizada no filtro */
  sprintf(expr, "udp and port 53");

  packetFilter(expr);

  return 0;
}

