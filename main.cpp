
#include <functional>
#include <iostream>
#include <map>

#include "mongoose.h"

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

/*
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

*/


class HTTPClient
{
    public:
        HTTPClient()
        {
            mg_mgr_init(&this->client, 0);
            // Save 'this' point to Mongoose manager
            // for later callback referencing.
            this->client.user_data = this;
        }
        ~HTTPClient()
        {
            this->client.user_data = 0;
            mg_mgr_free(&this->client);
        }

        typedef std::function<void(const std::string &)> Callback;

        // Only does GET requests.
        // TODO Introduce 'type' to perform POST requests as well.
        void request(const std::string &url, Callback success, Callback failure)
        {
            mg_connect_http(&this->client, handleEvent, url.c_str(), 0, 0);
            this->requests.insert(
                std::pair<std::string, RequestCallbacks>(
                    url, {success, failure}
                )
            );
        }

    private:
        struct RequestCallbacks
        {
            /*
            Request(
                const std::string &url,
                Callback success,
                Callback failure
            ) :
                url(url),
                success(success),
                failure(failure)
            { }
            */
                    
            Callback success;
            Callback failure;
        };

        mg_mgr client;
        std::map<std::string, RequestCallbacks> requests;

        static void handleEvent(mg_connection *conn, int event, void *data)
        {
            auto msg = static_cast<http_message *>(data);
            /*
            switch (event)
            {
                case MG_EV_CONNECT:
                    if (*(int *)data != 0)
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
            */

        }
};

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s URL\n", argv[0]);
        return 1;
    }
    auto url = argv[1];

    HTTPClient client;
    client.request(
        url,
        [&](const std::string &body) {
            printf("Received HTTP reply: '%s'", body.c_str());
        },
        [&](const std::string &reason) {
            printf("ERROR: '%s'", reason.c_str());
        }
    );
    /*
    while (client.needsProcessing())
    {
        client.process();
    }
    */

    return 0;
}

