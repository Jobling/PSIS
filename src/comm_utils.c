#include "comm_utils.h"

/* Function used to safely send all the data
 *
 * This function returns the total bytes sent in case of success
 * This function returns -1 in case of error */
int kv_send(int sock, void * buf, size_t len){
    int total_bytes_sent;
    int bytes_sent;
    void * ptr = buf;

    total_bytes_sent = bytes_sent = 0;

    while(len > 0){
        bytes_sent = send(sock, ptr, len, 0);
        if(bytes_sent < 0)
            return bytes_sent;
        else{
            total_bytes_sent += bytes_sent;
            ptr += bytes_sent;
            len -= bytes_sent;
        }
    }
    return total_bytes_sent;
}

/* Function used to safely receive all the data
 *
 * This function returns the total bytes read in case of success
 * This function returns 0 in case the socket is closed
 * This function returns -1 in case some error occurs */
int kv_recv(int sock, void * buf, size_t len){
    int total_bytes_read;
    int bytes_read;
    void * ptr = buf;

    total_bytes_read = bytes_read = 0;

    while(len > 0){
        bytes_read = recv(sock, buf, len, 0);
        if(bytes_read <= 0)
            return bytes_read;
        else{
            total_bytes_read += bytes_read;
            ptr += bytes_read;
            len -= bytes_read;
        }
    }
    return total_bytes_read;
}
