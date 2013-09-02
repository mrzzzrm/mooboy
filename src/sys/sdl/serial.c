#include "core/serial.h"
#include "sys/sys.h"
#include "core/serial.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <SDL/SDL.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>


#define MSG_INTERNAL_PERIOD 0x00
#define MSG_DATA 0x01

typedef struct msg_s {
    u8 data;
    struct msg_s *prev, *next;
} msg_t;


typedef struct {
    msg_t *first, *last;
} msgstream_t;

static int s = -1;

static u8 in_byte;
static u8 out_byte;
static u8 in_byte_shifts;

static msgstream_t instream;


static u8 stream_pop(msgstream_t *stream) {
    assert(stream->first != NULL);

    msg_t *msg = stream->first;
    u8 data = msg->data;

    if(msg->next != NULL) {
        msg->next->prev = NULL;
    }
    else {
        stream->last = NULL;
    }
    stream->first = msg->next;

    free(msg);

    return data;
}

static void stream_push(msgstream_t *stream, u8 data) {
    msg_t *msg = malloc(sizeof(*msg));
    msg->data = data;
    msg->next = NULL;
    stream->last = msg;

    if(stream->first == NULL) {
        msg->prev = NULL;
        stream->first = msg;
    }
    else {
        msg->prev = stream->last;
        stream->last->next = msg;
    }
}

static int stream_empty(msgstream_t *stream) {
    return stream->first == NULL;
}
//
//static int stream_len(msgstream_t *stream) {
//    int l = 0;
//    msg_t *m;
//
//    for(m = stream->first; m != NULL; m = m->next) {
//        l++;
//    }
//
//    return l;
//}
//
//static void stream_tobuf(msgstream_t *stream, u8 *buf) {
//    int b;
//    msg_t *m;
//
//    for(m = stream->first, b = 0; m != NULL; m = m->next) {
//        buf[b++] = m->data;
//    }
//}

static void parse_msgs(u8 *buf, ssize_t size) {
    int c;

    for(c = 0; c < size; c++) {
        switch(buf[c]) {
            case MSG_INTERNAL_PERIOD:
                serial.external_period = ntohl(*((u32*)&buf[c+1]));
                c += 4;
                //printf("Net: Received external_period: %i\n", serial.external_period);
            break;
            case MSG_DATA:
                stream_push(&instream, buf[c+1]);
                c++;
                //printf("Net: Received data: %.2X\n", buf[c]);
            break;
            default:
                printf("No such msg-type %.2X\n", buf[c]);
        }
    }
}

static void receive_msgs() {
    u8 buf[1024];
    ssize_t received;

    received = recv(s, buf, sizeof(buf), 0);
    if(received < 0) {
        return;
    }
    if(received > sizeof(buf)) {
        //printf("Net: Input buffer overflow %i > %i\n", received, sizeof(buf));
        assert(0);
    }

    parse_msgs(buf, received);
}

void sys_serial_init() {
    memset(&instream, 0x00, sizeof(instream));

    in_byte = 0;
    out_byte = 0;
    in_byte_shifts = 0;
}

void sys_serial_connect() {
    struct sockaddr_in addr;

    s = socket(AF_INET, SOCK_STREAM, 0);
    assert(s >= 0);


    addr.sin_family = AF_INET;
    addr.sin_port = htons(55876);
    inet_aton ("127.0.0.1", &addr.sin_addr);
    assert(connect(s, (struct sockaddr *)&addr, sizeof(addr)) == 0);

    int flags = fcntl(s, F_GETFL, 0);
    assert(flags != -1);
    fcntl(s, F_SETFL, flags | O_NONBLOCK);

    printf("Client ready!\n");
    sys_serial_update_internal_period();
}

void sys_serial_step() {
    if(s < 0) {
        return;
    }

    //send_msgs();
    receive_msgs();
}

int sys_serial_incoming() {
    receive_msgs();
    return !stream_empty(&instream) || (in_byte_shifts < 8);
}

void sys_serial_out_bit(int bit) {
    out_byte <<= 1;
    out_byte |= bit;
}

int sys_serial_in_bit() {
    if(in_byte_shifts == 8) {
        in_byte_shifts = 0;
        in_byte = stream_pop(&instream);
        printf(">> %.2X [%i %i]\n", in_byte, SDL_GetTicks() % 1000, serial.sc & 0x01);
    }

    int bit = in_byte & 0x80 ? 1 : 0;
    in_byte <<= 1;
    in_byte_shifts++;

    return bit;
}

void sys_serial_update_internal_period() {
    u8 msg[5];

    if(s < 0) {
        return;
    }

    msg[0] = MSG_INTERNAL_PERIOD;
    *(u32*)(&msg[1]) = htonl(serial.internal_period);

    send(s, msg, sizeof(msg), 0);

    //printf("Net: Sending internal_period %i\n", serial.internal_period);
}

void sys_serial_transfer_complete() {
    u8 msg[2];

    msg[0] = MSG_DATA;
    msg[1] = out_byte;

    send(s, msg, sizeof(msg), 0);

    printf("<< %.2X [%i %i]\n", out_byte, SDL_GetTicks() % 1000, serial.sc & 0x01);
}

