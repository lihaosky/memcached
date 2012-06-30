#include "monoisis.h"


void isis_init(int nnum, int ssize, int mrank) {
	MonoMethod *m;
	void *iter;
	
	node_num = nnum;
	shard_size = ssize;
	my_rank = mrank;
	
	domain = mono_jit_init("MonoIsis.exe");
	mono_config_parse(NULL);
	assembly = mono_domain_assembly_open(domain, "MonoIsis.exe");
	if (!assembly) {
		fprintf(stderr, "Fail to open assembly MonoIsis.exe");
		exit;
	}
	
	image = mono_assembly_get_image(assembly);
	class = mono_class_from_name(image, "IsisService", "IsisServer");
	
	while ((m = mono_class_get_methods(class, &iter))) {
		printf("Found a method: %s\n", mono_method_get_name(m));
		
		if (!strcmp(mono_method_get_name(m), "createGroup")) {
			start_method = m;
		}
		if (!strcmp(mono_method_get_name(m), "commandSend")) {
			send_method = m;
		}
		if (!strcmp(mono_method_get_name(m), "isStarted")) {
			is_started_method = m;
		}
	}
}

/* Thread function */
void* isis_start_fun() {
	void *args[3];
	
	/* Must be called if a new thread is using mono */
	mono_thread_attach(domain);
	
	/* Set up parameters */
	args[0] = &node_num;
	args[1] = &shard_size;
	args[2] = &my_rank;
	
	/* Call createGroup method in C# */
	mono_runtime_invoke(start_method, NULL, args, NULL);
}

/* Start ISIS */
void isis_start() {
	pthread_t thread;
	
	/* Start ISIS */
	pthread_create(&thread, NULL, isis_start_fun, NULL);
}

void safe_send(const char *command, int rank) {
	MonoString *cmd;
	void *args[2];
	
	
	
	cmd = mono_string_new(domain, command);
	args[0] = cmd;
	args[1] = &rank;
	
	mono_runtime_invoke(send_message, NULL, args, NULL);
}

int main(int argc, char **argv) {
	while (ret == 0) {
	result = mono_runtime_invoke(is_started, NULL, NULL, NULL);
	ret = *(int*)mono_object_unbox(result);
	printf("return value is %d\n", ret);
	sleep(3);
	}
	safe_send("insert li", 10);
}
