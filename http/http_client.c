#include <stdio.h>   
#include <stdlib.h>  
#include <string.h>  
#include <sys/socket.h> // Socket programming (socket, connect)
#include <netdb.h>   // DNS resolution (getaddrinfo)
#include <unistd.h>  // POSIX API (close)
#include <arpa/inet.h> // Internet operations (inet_ntop)

const char* get_filename_from_path(const char* path){
    if(!path || *path=='\0' || strcmp(path,"/") == 0) return "index.html";

    const char* last_slash = strrchr(path,'/');
    if(!last_slash) return path;

    if(*(last_slash+1) == '\0') return "index.html";

    return last_slash +1;
}

int parse_url(const char *url, char **host, char **port, char **path){
    // 1. Check if URL starts with http://
    if(strncmp(url, "http://",7) != 0){
        fprintf(stderr, "Error: URL must start with http://");
        return -1;
    } 

    // 2. Extract the "authority" part (host:port) and the path
    const char *authority_start = url + 7; // Skip "http://"
    const char *path_start = strchr(authority_start, '/');
    size_t authority_len;
    
    if(path_start){
        // If '/' exists, the path starts there
        authority_len = path_start - authority_start;
        *path = strdup(path_start); // Copy the path "/img.jpg" 
    } else{
        authority_len = strlen(authority_start);
        *path = strdup("/"); // Default path is the root
    } 

    // 3. Extract the host and port from the "authority"
    char *authority = strndup(authority_start, authority_len);
    if(!authority){
        perror("strndup failed");
        return -1;
    }

    // 4. Look for ':' to separate host and port
    char *colon = strchr(authority, ':');
    if(colon){
        // If ':' exists, split the string
        *colon = '\0'; // "host:port" -> "host\0port"
        *host = strdup(authority);
        *port = strdup(colon+1);
    } else{
        *host = strdup(authority);
        *port = strdup("80"); // Default HTTP port 
    }

    free(authority);
    // 5. Check if all allocations succeeded
    if(!*host||!*port||!*path){
        free(*host);
        free(*port);
        free(*path);
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[]){

    if(argc < 2){
        fprintf(stderr, "Error: please provide a URL.\n");
        fprintf(stderr, "Usage: %s <url>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // 1. URL Parsing
    char *url = argv[1];
    char *host = NULL, *port = NULL, *path = NULL;
    // Parse the URL
    if(parse_url(url, &host, &port, &path) != 0){
        fprintf(stderr, "Failed to parse URL\n");
        exit(EXIT_FAILURE);
    }

    // 2. DNS Resolution
    struct addrinfo hints, *res;
    int status;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP socket

    // Perform DNS lookup -> transform "www.google.com" into an IP
    status = getaddrinfo(host, port, &hints, &res);
    if(status != 0){
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        goto cleanup;
    }

    // 3. Create socket and connect
    int sockfd = -1;
    struct addrinfo *p;
    for(p=res;p!=NULL;p = p->ai_next){
        // Create socket
        sockfd = socket(p->ai_family,p->ai_socktype,p->ai_protocol);
        if(sockfd == -1) {
            perror("client: socket");
            continue;
        }
        
        // Connect to the server
        if(connect(sockfd,p->ai_addr,p->ai_addrlen)==-1){
            close(sockfd);
            perror("client: connect");
            continue;
        }
        break;
    }

    // Go through the entire list and don't connect with anyone
    if(p == NULL){
        fprintf(stderr,"Failed to connect to any address\n");
        goto cleanup;
    }

    // 4. Format HTTP Request
    char request[1024];
    int req_len = snprintf(request, sizeof(request),
                "GET %s HTTP/1.1\r\n"
                "Host: %s\r\n"
                "Connection: close\r\n"
                "\r\n",
                path,host);
    
    if(req_len<0||(size_t)req_len >= sizeof(request)){
        fprintf (stderr, "Request too long\n");
        close(sockfd);
        goto cleanup;
    }

    // 5. Send Request
    if(send(sockfd, request,req_len,0) == -1){
        perror("Send failed");
        close(sockfd);
        goto cleanup;
    }

    // 6. Receive Response
    char header_buffer[8192];
    char *header_end = NULL;
    ssize_t total_header_bytes = 0;
    ssize_t bytes_received;

    // Read data from the socket untik the server closes the connection
    while((bytes_received = recv(sockfd, header_buffer + total_header_bytes, sizeof(header_buffer)- total_header_bytes-1,0))>0){
        total_header_bytes += bytes_received;
        header_buffer[total_header_bytes] = '\0';

        header_end = strstr(header_buffer, "\r\n\r\n");
        if(header_end) break;

        if(total_header_bytes>=sizeof(header_buffer)-1){
            fprintf(stderr, "Error: Headers too large");
            goto cleanup;
        }
    }

    if(bytes_received<=0){
        perror("recv failed");
    }

    // 7. Parse Headers
    *header_end = '\0';
    char *body_start = header_end+4; // Skip the "\r\n"
    ssize_t body_chuck_len = total_header_bytes-(body_start-header_buffer);
    if(strncmp(header_buffer, "HTTP/1.1 200 OK",15)!=0&&strncmp(header_buffer,"HTTP/1.0 200 OK", 15)!=0){
        char *first_line_end = strstr(header_buffer, "\r\n");
        if(first_line_end) *first_line_end='\0';

        fprintf(stderr, "Error: File unavailabe. Server respond: %s\n", header_buffer);
        goto cleanup;
    }

    // 8. Save File
    FILE *outfile = NULL;
    const char *filename = get_filename_from_path(path);
    printf("Recebendo 200 OK. Salvando em: %s\n", filename);
    outfile = fopen(filename,"wb");
    if(!outfile){
        perror("Error creating the file");
        goto cleanup;
    }
    if(body_chuck_len>0){
        fwrite(body_start,1,body_chuck_len,outfile);
    }

    char body_buffer[4096];
    while((bytes_received = recv(sockfd, body_buffer, sizeof(body_buffer),0))>0){
        fwrite(body_buffer,1,bytes_received,outfile);
    }

    fclose(outfile);
    outfile=NULL;
    close(sockfd);
    cleanup:
    free(host);
    free(port);
    free(path);
    if(res) freeaddrinfo(res);
    if (sockfd != -1) close(sockfd); // Fecha o socket se estiver aberto
    if (outfile) fclose(outfile);

    return EXIT_SUCCESS;
}