#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 8080
#define BUFFER_SIZE 1024

volatile sig_atomic_t was_sighup = 0;

static void sighup_handler(int signal_number) {
	(void) signal_number;
	was_sighup = 1;
}

int main() {
	int server_fd, client_fd = -1, max_fd;
	struct sockaddr_in address;
	int opt = 1;

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) {
		perror("socket");
		return EXIT_FAILURE;
	}

	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
		perror("setsockopt");
		close(server_fd);
		return EXIT_FAILURE;
	}

	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(PORT);

	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
		perror("bind");
		close(server_fd);
		return EXIT_FAILURE;
	}

	if (listen(server_fd, 16) == -1) {
		perror("listen");
		close(server_fd);
		return EXIT_FAILURE;
	}

	printf("Server listening on port %d...\n", PORT);

	struct sigaction action;
	memset(&action, 0, sizeof(action));
	action.sa_handler = sighup_handler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;

	if (sigaction(SIGHUP, &action, NULL) == -1) {
		perror("sigaction");
		close(server_fd);
		return EXIT_FAILURE;
	}

	sigset_t blocked_mask, original_mask;
	sigemptyset(&blocked_mask);
	sigaddset(&blocked_mask, SIGHUP);

	if (sigprocmask(SIG_BLOCK, &blocked_mask, &original_mask) == -1) {
		perror("sigprocmask");
		close(server_fd);
		return EXIT_FAILURE;
	}

	max_fd = server_fd;

	while (1) {
		fd_set read_fds;
		FD_ZERO(&read_fds);
		FD_SET(server_fd, &read_fds);

		if (client_fd != -1) {
			FD_SET(client_fd, &read_fds);
		}

		int activity = pselect(max_fd + 1, &read_fds, NULL, NULL, NULL, &original_mask);
		if (activity < 0) {
			if (errno == EINTR) {
			} else {
				perror("pselect");
				break;
			}
		}

		if (was_sighup) {
			printf("\n[EVENT] Received SIGHUP signal!\n");
			was_sighup = 0;
		}

		if (FD_ISSET(server_fd, &read_fds)) {
			int new_client_fd = accept(server_fd, NULL, NULL);
			if (new_client_fd != -1) {
				if (client_fd == -1) {
					client_fd = new_client_fd;
					if (client_fd > max_fd) {
						max_fd = client_fd;
					}
					printf("[EVEN] New client connected and accepted.\n");
				} else {
					printf("[EVEN] Extra client connected, closing immediately.\n");
					close(new_client_fd);
				}
			}
		}

		if (client_fd != -1 && FD_ISSET(client_fd, &read_fds)) {
			char buffer[BUFFER_SIZE];
			ssize_t received = recv(client_fd, buffer, sizeof(buffer), 0);

			if (received > 0) {
				printf("[EVEN] Received %zd bytes from client.\n", received);
			} else if (received == 0) {
				printf("[EVEN] Client disconnected gracefully./n");
				close(client_fd);
				client_fd = -1;
				max_fd = server_fd;
			} else {
				perror("[ERROR] recv");
				close(client_fd);
				client_fd = -1;
				max_fd = server_fd;
			}
		}
	}

	if (client_fd != -1) {
		close(client_fd);
	}
	close(server_fd);
	sigprocmask(SIG_SETMASK, &original_mask, NULL);

	return EXIT_SUCCESS;
}
