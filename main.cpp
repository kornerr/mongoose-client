
#include "mongoose.h"

#include <iostream>

static int s_exit_flag = 0;

static void ev_handler(mg_connection *nc, int ev, void *ev_data)
{
    struct http_message *hm = (http_message *)ev_data;

    switch (ev)
    {
        case MG_EV_CONNECT:
            if (*(int *) ev_data != 0)
            {
                fprintf(stderr, "connect() failed: %s\n", strerror(*(int *) ev_data));
                s_exit_flag = 1;
            }
            break;
        case MG_EV_HTTP_REPLY:
            {
                nc->flags |= MG_F_CLOSE_IMMEDIATELY;
                auto body = std::string(hm->body.p, hm->body.len);
                printf("HTTP reply: '%s'\n", body.c_str());
                s_exit_flag = 1;
                break;
            }
        case MG_EV_CLOSE:
            if (s_exit_flag == 0)
            {
                printf("Server closed connection\n");
                s_exit_flag = 1;
            }
            break;
        default:
            break;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <URL>\n", argv[0]);
        return 1;
    }
    auto url = argv[1];

    mg_mgr mgr;
    mg_mgr_init(&mgr, 0);

    mg_connect_http(&mgr, ev_handler, url, 0, 0);

    while (s_exit_flag == 0)
    {
        mg_mgr_poll(&mgr, 1000);
        std::cout << "Checking\n";
    }
    mg_mgr_free(&mgr);

    return 0;
}

