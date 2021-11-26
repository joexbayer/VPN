#include "server.h"
#include "vpn_registry.h"
#include "vpn_config.h"

pthread_t tid[2];
int counter;

void stop_server()
{
	printf("\nStopped.\n");
	exit(EXIT_SUCCESS);
}

void* doSomeThing(void *arg)
{
    while(1){

    }
}

int main(int argc, char const *argv[])
{
	signal(SIGINT, stop_server);

	/* Create a new VPN registry */
	struct vpn_registry* registry = create_registry((uint8_t*) "10.0.0.1/24");

	free_vpn_registry(registry);

	/* Create thread for incomming client packets */
    int err;
    err = pthread_create(&(tid[0]), NULL, &doSomeThing, NULL);
    if(err != 0){
		printf("\ncan't create thread :[%s]", strerror(err));
    }


    /* Create thread for incomming tun packets */
	err = pthread_create(&(tid[1]), NULL, &doSomeThing, NULL);
    if(err != 0){
		printf("\ncan't create thread :[%s]", strerror(err));
    }

    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);

	/* code */
	return 0;
}