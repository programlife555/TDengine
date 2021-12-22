#include <stdio.h>
#include <raft.h>
#include <raft/uv.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <getopt.h>
#include "raftServer.h"

#define COMMAND_LEN 128

const char *exe_name;

void *startServerFunc(void *param) {
	SRaftServer *pServer = (SRaftServer*)param;
	int32_t r = raftServerStart(pServer);
	assert(r == 0);

	return NULL;
}

// Console ---------------------------------
void console(SRaftServer *pRaftServer) {
	while (1) {
		char cmd_buf[COMMAND_LEN];
		memset(cmd_buf, 0, sizeof(cmd_buf));
		char *ret = fgets(cmd_buf, COMMAND_LEN, stdin);
		if (!ret) {
			exit(-1);
		}

		int pos = strlen(cmd_buf);                  
		if(cmd_buf[pos - 1] == '\n') {
			cmd_buf[pos - 1] = '\0';
		}

		if (strncmp(cmd_buf, "", COMMAND_LEN) == 0) {
			continue;
		}

		printf("cmd_buf: [%s] \n", cmd_buf);
	}
}

void *startConsoleFunc(void *param) {
	SRaftServer *pServer = (SRaftServer*)param;
	console(pServer);
	return NULL;
}

// Config ---------------------------------
#define DIR_LEN 128
#define HOST_LEN 128
typedef struct SRaftServerConfig {
	char host[HOST_LEN];
	uint32_t port;
	char dir[DIR_LEN];
} SRaftServerConfig;

void parseConf(int argc, char **argv, SRaftServerConfig *pConf) {
	snprintf(pConf->dir, sizeof(pConf->dir), "%s", argv[1]);
	sscanf(argv[2], "%u", &pConf->port);
	snprintf(pConf->dir, sizeof(pConf->dir), "%s", argv[3]);
}

// -----------------------------------------
void usage() {
	printf("\n");
	printf("usage: %s host port dir \n", exe_name);
	printf("eg   : %s 127.0.0.1 10000 ./data \n", exe_name);
	printf("\n");
}

int main(int argc, char **argv) { 
	srand(time(NULL));
	int32_t ret;

	exe_name = argv[0];
	if (argc != 4) {
		usage();
		exit(-1);
	}

	SRaftServerConfig conf;
	parseConf(argc, argv, &conf);
	
	struct raft_fsm fsm;
	initFsm(&fsm);

	SRaftServer raftServer;
	ret = raftServerInit(&raftServer, &conf, &fsm);

	pthread_t tidRaftServer;
	pthread_create(&tidRaftServer, NULL, startServerFunc, &raftServer);

	pthread_t tidConsole;
	pthread_create(&tidConsole, NULL, startConsoleFunc, &raftServer);

	while (1) {
		sleep(10);
	}

	return 0;
}
