#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>

#define FALSE 0
#define TRUE 1
#define BUFSIZE 1024
#define PORT 8000
#define TEMPLATE "template.json"
#define PATTERN "{\n\t\"response\": %%s\n}"

static int verbose = FALSE;

static int create_server(int port, char *data) {
	int rc = EXIT_SUCCESS;
	int create_socket = 0, new_socket = 0;
	size_t bufsize = BUFSIZE;
	char *buffer = NULL;
	socklen_t addrlen;
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_addr.s_addr = INADDR_ANY,
		.sin_port = htons(port)
	};

	buffer = malloc(bufsize);

	if ((create_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		printf("Socket could not be created\n");

		rc = EXIT_FAILURE;
		goto cleanup;
	}

	if (bind(create_socket, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
		printf("Could not bind socket\n");

		rc = EXIT_FAILURE;
		goto cleanup;
	}

	printf("Server listening on port %d\n", port);

	while (1 == TRUE) {
		if (listen(create_socket, 10) < 0) {
			printf("Server: listen\n");

			rc = EXIT_FAILURE;
			goto cleanup;
		}

		if ((new_socket = accept(create_socket, (struct sockaddr *)&addr, &addrlen)) < 0) {
			printf("Server: accept\n");

			rc = EXIT_FAILURE;
			goto cleanup;
		}

		if (new_socket == 0) {
			printf("The client coult not connect\n");

			rc = EXIT_FAILURE;
			goto cleanup;
		}

		recv(new_socket, buffer, bufsize, 0);
		write(new_socket, data, (int)strlen(data));
		close(new_socket);

		if (verbose == TRUE) {
			printf("%s\n", buffer);
		}
	}

cleanup:
	close(create_socket);

	if (buffer) {
		free(buffer);
	}

	return rc;
}

static void usage(char *exec) {
	int length = (int)strlen(exec);

	printf("%s (<port>)\n", exec);
	printf("%*s (-t|--template <template.json>)\n", length, "");
	printf("%*s (-v|--verbose)\n", length, "");
	printf("%*s (-h|--help)\n", length, "");
}

static int check_mem(char *mem) {
	if (!mem) {
		printf("Memory error\n");

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

static size_t num_digits(int num) {
	size_t i = 0, l = 0;

	for (i = 5, l = 0; i >= l; i--) {
		if (num >= (int)pow(10, (double)i)) {
			return i + 1;
		}
	}

	return 1;
}

static int is_christmas() {
	int christmas = 0;
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	christmas = (tm.tm_mon == 11 && tm.tm_mday == 25) ? 1 : 0;

	if (verbose == TRUE) {
		printf("Today is %d/%d. ", tm.tm_mon + 1, tm.tm_mday);
		printf("It is %sChristmas.\n", christmas == 1 ? "" : "not ");
	}
	
	return christmas;
}

int main(int argc, char *argv[]) {
	int rc = EXIT_SUCCESS, port = PORT, content_length = 0, christmas = 0;
	int i = 0, l = 0;
	size_t content_length_length = 0, filesize = 0;
	char *exec = NULL, *action = NULL, *data = NULL, *response = NULL;
	char *pattern = NULL, *filename = NULL;
	FILE *json = NULL;

	exec = argv[0];

	if (argc > 1) {
		for (i = 1, l = argc; i < l; i++) {
			action = argv[i];

			if (strcmp(action, "-t") == 0 || strcmp(action, "--template") == 0) {
				filename = argv[++i];
			} else if (strcmp(action, "-v") == 0 || strcmp(action, "--verbose") == 0) {
				verbose = TRUE;
			} else if (strcmp(action, "-h") == 0 || strcmp(action, "--help") == 0) {
				usage(exec);

				rc = EXIT_FAILURE;
				goto cleanup;
			} else {
				if (strncmp(action, "-", 1) == 0) {
					printf("Unknown action %s\n", action);

					rc = EXIT_FAILURE;
					goto cleanup;
				}

				port = atoi(action);
			}
		}
	}

	if (!filename) {
		filename = TEMPLATE;
	}

	json = fopen(filename, "rb");

	if (json) {
		(void)fseek(json, 0, SEEK_END);
		filesize = (size_t)ftell(json);
		(void)fseek(json, 0, SEEK_SET);

		if (filesize == 0) {
			printf("%s is empty\n", filename);

			rc = EXIT_FAILURE;
			goto cleanup;
		}

		pattern = malloc(sizeof(char) * filesize + 1);

		if (check_mem(pattern) == TRUE) {
			rc = EXIT_FAILURE;
			goto cleanup;
		}

		if (fread(pattern, 1, filesize, json) != filesize) {
			printf("Could not read %s\n", filename);

			rc = EXIT_FAILURE;
			goto cleanup;
		}
	} else {
		pattern = malloc(sizeof(char) * 19);

		if (check_mem(pattern) == TRUE) {
			rc = EXIT_FAILURE;
			goto cleanup;
		}

		(void)snprintf(pattern, strlen(PATTERN) + 1, PATTERN);
	}

	christmas = is_christmas();

	response = malloc(sizeof(char) * 5);

	if (check_mem(response) == TRUE) {
		rc = EXIT_FAILURE;
		goto cleanup;
	}

	(void)snprintf(response, (size_t)(christmas == TRUE ? 4 : 5) + 1, christmas == TRUE ? "true" : "false");

	content_length = (int)(strlen(pattern) - 2 + strlen(response));
	content_length_length = num_digits(content_length);

	data = malloc(sizeof(char) * (68 + content_length + content_length_length) + 1);

	if (check_mem(data) == TRUE) {
		rc = EXIT_FAILURE;
		goto cleanup;
	}

	(void)snprintf(data, 17, "HTTP/1.1 200 OK\n");
	(void)snprintf(data + strlen(data), 18 + content_length_length, "Content-length: %d\n", content_length);
	(void)snprintf(data + strlen(data), 33, "Content-Type: application/json\n\n");
	(void)snprintf(data + strlen(data), strlen(pattern) - 1 + ((christmas == TRUE) ? 4 : 5), pattern, response);

	if (create_server(port, data) != 0) {
		printf("Exiting...\n");

		rc = EXIT_FAILURE;
		goto cleanup;
	}

cleanup:
	if (data) {
		free(data);
	}

	if (response) {
		free(response);
	}

	if (pattern) {
		free(pattern);
	}

	if (json) {
		(void)fclose(json);
	}

	return rc;
}