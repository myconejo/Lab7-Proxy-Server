#include <stdio.h>
#include "csapp.h"
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
void sigchld_handler(int sig) {while (waitpid(-1, NULL, WUNTRACED|WNOHANG)>0) ;}
void proxy(int proxy_connfd){
    int proxy_clientfd, n;                          // proxy connecting to the server, n for size to read
    rio_t rio_server, rio_client;                   // rio for sending/receiving request & response
    char buf[MAXLINE],  method[MAXLINE], uri[MAXLINE], version[MAXLINE], hostname[MAXLINE], port[MAXLINE], request[MAXLINE];
    Rio_readinitb(&rio_client, proxy_connfd);       /* (1) Read the client request */
    Rio_readlineb(&rio_client, buf, MAXLINE);       // Read client's request to the buf
    sscanf(buf, "%s %s %s", method, uri, version);  // Request parsed: method, uri, version
    char* slash = index(uri + 7, '/');              // uri + 7: ignore 'http://'
    char* colon = index(uri + 7, ':');
    *slash = '\0';                                  /* (2) PARSE the uri */
    if (colon == NULL) { strcpy(port, "80");        // Default port = 80
        strcpy(hostname, uri + 7);                  // Get hostname
    } else { strcpy(port, colon+1);
        *colon = '\0';
        strcpy(hostname, uri + 7);                  // Get hostname
        *colon = ':';
    } *slash = '/';
    strcpy(request, slash);                         // Get request
    proxy_clientfd = Open_clientfd(hostname, port); /* (3) Connect server */
    sprintf(buf, "%s %s %s\r\n", method, request, "HTTP/1.0"); 
    sprintf(buf, "%sHost: %s\r\n", buf, hostname);
    sprintf(buf, "%s%s",buf,user_agent_hdr);        /* (4) HTTP Request Header */
    sprintf(buf, "%sConnection: close\r\nProxy-Connection: close\r\n\r\n", buf);
    Rio_readinitb(&rio_server, proxy_clientfd);     // Associate proxy-server fd to rio_server
    Rio_writen(proxy_clientfd, buf, MAXLINE);       /* (5) Send HTTP request Header to end server */
    while((n = (int)Rio_readlineb(&rio_server, buf, MAXLINE)) > 0) { 
        Rio_writen(proxy_connfd, buf, n);           /* (6) Send end server's response to the client */
    } Close(proxy_clientfd);                        /* (7) Close: proxy and server connection */ 
}
int main(int argc, char **argv) {               
    int proxy_listenfd, proxy_connfd;               // fd for client - proxy listen and connection
    socklen_t clientlen;                            // size of sockaddr_storage
    struct sockaddr_storage clientaddr;             // socket information of the client
    Signal(SIGCHLD, sigchld_handler);               /* (2) Reap zombies (See Ch12 Figure 12.5) */
    proxy_listenfd = Open_listenfd(argv[1]);        /* (3) Proxy listens incoming client connections */
    while (1) {                                     // Wait for incoming client requests
        clientlen = sizeof(clientaddr);             /* (4) Proxy accepts the client request */
        proxy_connfd = Accept(proxy_listenfd, (SA *)&clientaddr, &clientlen);
        if (Fork() == 0){                           /* (5) Fork for concurrency */
            proxy(proxy_connfd);                    // Handle the client request
            exit(0);                                // Terminate child process
        } Close(proxy_connfd);                      /* (6) Close the parent proxy connfd */
    }                                               // Close is important for memory leak prevention 
}
