#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *conn_hdr = "Connection: close\r\n";
static const char *prox_hdr = "Proxy-Connection: close\r\n";

typedef struct {
    char* uri;
    char* obj;
    int cnt;
    int flag;
    int time;
    
    sem_t mutex;
    sem_t w;
} Cache; 

typedef struct {
    int **buf;        
    int n;      
    int front;        
    int rear;         
    sem_t mutex;      
    sem_t slots;     
    sem_t items;      
} sbuf_t;

void sbuf_init(sbuf_t *sp, int n);
void sbuf_deinit(sbuf_t *sp);
void sbuf_insert(sbuf_t *sp, int* item);
int sbuf_remove(sbuf_t *sp);
void doit(int fd);
void parse_uri(char *uri, char *host, char *path, char *port);
void* thread(void* vargp);
void init_Cache(void);
void update_LRU(int index);
int get_index(void);
void write_Cache(char* uri, char* obj);
int find_Cache(char* uri);
void free_Cache(void);
void build_header(rio_t *rp, char *req, char* path, char *host, char* port);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

Cache cache[10];
sbuf_t sbuf;

int main(int argc, char **argv) {
    pthread_t tid;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    if (argc != 2) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(1);
    }

    init_Cache();
    signal(SIGPIPE, SIG_IGN);
    int listenfd = Open_listenfd(argv[1]);
    
    sbuf_init(&sbuf, 16);
    
    for (int i = 0; i < 4; i++)
    {
        Pthread_create(&tid, NULL, thread, NULL);
    }
    
    while (1) {
	clientlen = sizeof(clientaddr);
	int* connfd = (int*)Malloc(sizeof(int));
	(*connfd) = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        sbuf_insert(&sbuf, connfd);                     
    }
    
    free_Cache();
    
    return 0;
}

void* thread(void* vargp) {
    Pthread_detach(Pthread_self());
    while (1)
    {
        int connfd = sbuf_remove(&sbuf);
        doit(connfd);
        Close(connfd);
    }
    
    return NULL;
}

void doit(int fd) {
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char host[MAXLINE], path[MAXLINE], port[MAXLINE];
    char req[MAXLINE];
    char uri_copy[MAXLINE];
    rio_t rio, server_rio;
    int i = 0;

    Rio_readinitb(&rio, fd);
    if (!Rio_readlineb(&rio, buf, MAXLINE)) 
        return;
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);  
    strcpy(uri_copy, uri);
    if (strcasecmp(method, "GET")) {                   
        clienterror(fd, method, "501", "Not Implemented", "Tiny does not implement this method");
        return;
    }
    
    if ((i = find_Cache(uri)) != -1) {
        P(&cache[i].mutex);
    	cache[i].cnt++;
    	if (cache[i].cnt == 1) {
    	    P(&cache[i].w);
    	}
    	V(&cache[i].mutex);
    	
    	Rio_writen(fd, cache[i].obj, strlen(cache[i].obj));
    	
    	P(&cache[i].mutex);
    	cache[i].cnt--;
    	if (cache[i].cnt == 0) {
    	    V(&cache[i].w);
    	}
    	V(&cache[i].mutex);
    
    	return;
    }               
    
    parse_uri(uri, host, path, port);
    
    build_header(&rio, req, path, host, port);
    
    int serverfd = Open_clientfd(host, port);
    
    if (serverfd < 0)
    {
        printf("connection failed\n");
        return;
    }
    
    Rio_readinitb(&server_rio, serverfd);
    Rio_writen(serverfd, req, strlen(req));

    size_t n;
    int size_buf = 0;
    char cache_buf[MAX_OBJECT_SIZE];
    memset(cache_buf, 0, sizeof(cache_buf)); 

    while ((n = Rio_readlineb(&server_rio, buf, MAXLINE)) != 0) {
	size_buf += n;
        if (size_buf < MAX_OBJECT_SIZE) {
	    strcat(cache_buf, buf);
	}
        printf("proxy received %d bytes, then send\n", (int)n);
        Rio_writen(fd, buf, n);
    }
    
    Close(serverfd);
    
    if (size_buf < MAX_OBJECT_SIZE){
        write_Cache(uri_copy, cache_buf);
    }
}

void init_Cache(void) {
    for (int i = 0; i < 10; i++) {
    	cache[i].uri = (char*)Malloc(sizeof(char) * MAXLINE);
    	cache[i].obj = (char*)Malloc(sizeof(char) * MAX_OBJECT_SIZE);
    	cache[i].cnt = 0;
    	cache[i].flag = 0;
    	cache[i].time = 0;
    	
    	Sem_init(&(cache[i].mutex), 0, 1);
    	Sem_init(&(cache[i].w), 0, 1);
    }
}

int find_Cache(char* uri) {
    for (int i = 0; i < 10; i++) {
    	P(&cache[i].mutex);
    	cache[i].cnt++;
    	if (cache[i].cnt == 1) {
    	    P(&cache[i].w);
    	}
    	V(&cache[i].mutex);
    	
    	if (cache[i].flag && !strcmp(uri, cache[i].uri)) {
            P(&cache[i].mutex);
    	    cache[i].cnt--;
    	    if (cache[i].cnt == 0) {
    	        V(&cache[i].w);
    	    }
    	    V(&cache[i].mutex);
    	    return i;
    	}
    	
    	P(&cache[i].mutex);
    	cache[i].cnt--;
    	if (cache[i].cnt == 0) {
    	    V(&cache[i].w);
    	}
    	V(&cache[i].mutex);
    }
    
    return -1;
}

void update_LRU(int index) {
    for (int i = 0; i < 10; i++) {
    	if (cache[i].flag && i != index) {
    	    P(&cache[i].w);
    	    cache[i].time--;
    	    V(&cache[i].w);
    	}
    }
}

int get_index(void) {
    int min = __INT_MAX__;
    int index = 0;
    
    for (int i = 0; i < 10; i++) {
        P(&cache[i].mutex);
    	cache[i].cnt++;
    	if (cache[i].cnt == 1) {
    	    P(&cache[i].w);
    	}
    	V(&cache[i].mutex);
    
    	if (!cache[i].flag) {
    	    P(&cache[i].mutex);
    	    cache[i].cnt--;
    	    if (cache[i].cnt == 0) {
    	        V(&cache[i].w);
    	    }
    	    V(&cache[i].mutex);
    	    return i;
    	} else if (cache[i].time < min) {
    	    min = cache[i].time;
    	    index = i;
    	}
    	
    	P(&cache[i].mutex);
    	cache[i].cnt--;
    	if (cache[i].cnt == 0) {
    	    V(&cache[i].w);
    	}
    	V(&cache[i].mutex);
    }
    
    return index;
}

void write_Cache(char* uri, char* obj) {
    int index = get_index();
    
    P(&cache[index].w);
    strcpy(cache[index].uri, uri);
    strcpy(cache[index].obj, obj);
    cache[index].flag = 1;
    cache[index].time = __INT_MAX__;
    update_LRU(index);
    V(&cache[index].w);
}

void free_Cache(void) {
    for (int i = 0; i < 10; i++) {
    	free(cache[i].uri);
    	free(cache[i].obj);
    }
}

void parse_uri(char *uri, char *host, char *path, char *port) {
    if (strstr(uri, "http://") != uri) {
        fprintf(stderr, "Error: invalid uri!\n");
        exit(0);
    }
    
    uri += strlen("http://");
    
    char* c = strstr(uri, ":");
    *c = '\0';
    strcpy(host, uri);
    
    uri = c + 1;
    c = strstr(uri, "/");
    *c = '\0';
    strcpy(port, uri);
    
    *c = '/';
    strcpy(path, c);
}

void build_header(rio_t *rp, char *req, char* path, char *host, char* port) {
    char buf[MAXLINE];
    
    sprintf(req, "GET %s HTTP/1.0\r\n", path);

    while(Rio_readlineb(rp, buf, MAXLINE) > 0) {          
    	if (!strcmp(buf, "\r\n")) break;
        if (strstr(buf,"Host:") != NULL) continue;
        if (strstr(buf,"User-Agent:") != NULL) continue;
        if (strstr(buf,"Connection:") != NULL) continue;
        if (strstr(buf,"Proxy-Connection:") != NULL) continue;
	sprintf(req,"%s%s", req, buf);
    }
    sprintf(req, "%sHost: %s:%s\r\n",req, host, port);
    sprintf(req, "%s%s%s%s", req, user_agent_hdr, conn_hdr, prox_hdr);
    sprintf(req,"%s\r\n",req);
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) {
    char buf[MAXLINE];

    /* Print the HTTP response headers */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n\r\n");
    Rio_writen(fd, buf, strlen(buf));

    /* Print the HTTP response body */
    sprintf(buf, "<html><title>Tiny Error</title>");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<body bgcolor=""ffffff"">\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<hr><em>The Tiny Web server</em>\r\n");
    Rio_writen(fd, buf, strlen(buf));
}

void sbuf_init(sbuf_t *sp, int n)
{
    sp->buf = Calloc(n, sizeof(int*)); 
    sp->n = n;              
    sp->front = sp->rear = 0;      
    Sem_init(&sp->mutex, 0, 1);      
    Sem_init(&sp->slots, 0, n);    
    Sem_init(&sp->items, 0, 0);    
}

void sbuf_deinit(sbuf_t *sp)
{
    Free(sp->buf);
}

void sbuf_insert(sbuf_t *sp, int* item)
{
    P(&sp->slots);                    
    P(&sp->mutex);         
    sp->buf[(++sp->rear)%(sp->n)] = item;   
    V(&sp->mutex);                       
    V(&sp->items);                         
}

int sbuf_remove(sbuf_t *sp)
{
    int item;
    P(&sp->items);         
    P(&sp->mutex);          
    item = *(sp->buf[(++sp->front)%(sp->n)]); 
    V(&sp->mutex);                 
    V(&sp->slots);                    
    return item;
}
