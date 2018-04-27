
#include <functional>
#include <iostream>
#include <map>

#include "mongoose.h"

class HTTPClient
{
    public:
        typedef std::function<void(std::string)> Callback;

        HTTPClient(Callback success, Callback failure) :
            success(success),
            failure(failure),
            inProgress(false)
        {
            mg_mgr_init(&this->client, 0);
        }
        ~HTTPClient()
        {
            mg_mgr_free(&this->client);
        }

        // Only does GET requests.
        // NOTE Introduce 'type' to perform POST requests as well?
        void request(const std::string &url)
        {
            auto connection = mg_connect_http(&this->client, handleEvent, url.c_str(), 0, 0);
            // Save 'this' point to the conection
            // for later referencing in callbacks.
            connection->user_data = this;
            this->inProgress = true;
        }

        bool needsProcessing()
        {
            return this->inProgress;
        }

        void process()
        {
            mg_mgr_poll(&this->client, 1000);
        }

    private:
        mg_mgr client;
        Callback success;
        Callback failure;
        bool inProgress;

        static void handleEvent(mg_connection *connection, int event, void *data)
        {
            auto instance = reinterpret_cast<HTTPClient *>(connection->user_data);
            switch (event)
            {
                case MG_EV_CONNECT:
                    {
                        auto status = *static_cast<int *>(data);
                        if (status != 0)
                        {
                            //instance->failure(strerror(status));
                            instance->failure("Failed to connect");
                            instance->inProgress = false;
                        }
                    }
                    break;
                case MG_EV_HTTP_REPLY:
                    {
                        connection->flags |= MG_F_CLOSE_IMMEDIATELY;
                        auto msg = static_cast<http_message *>(data);
                        auto body = std::string(msg->body.p, msg->body.len);
                        instance->inProgress = false;
                        instance->success(body);
                    }
                    break;
                case MG_EV_CLOSE:
                    // Only report failure if CLOSE event precedes REPLY one.
                    if (instance->inProgress)
                    {
                        instance->failure("Server closed connection");
                    }
                    instance->inProgress = false;
                    break;
                default:
                    break;
            }
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

    auto successCallback = [=](std::string body) {
        printf("success\n");
        printf("Received HTTP reply: '%s'\n", body.c_str());
    };
    auto failureCallback = [=](std::string reason) {
        printf("failure\n");
        printf("ERROR Failed to perform request: '%s'\n", reason.c_str());
    };

    HTTPClient client(successCallback, failureCallback);
    client.request(url);

    while (client.needsProcessing())
    {
        client.process();
    }

    return 0;
}

