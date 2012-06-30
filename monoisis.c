#include <mono/jit/jit.h>
#include <mono/metadata/object.h>
#include <mono/metadata/environment.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

#include "memcached.h"

/* ISIS parameters */
static int node_num = -1;      /* Total node number */
static int shard_size = -1;    /* Shard size */
static int my_rank = -1;       /* My rank */

/* Libmono */
static MonoDomain *domain;     
static MonoAssembly *assembly;
static MonoImage *image;
static MonoClass *klass;
static MonoMethod *start_method;  /* createGroup method */
static MonoMethod *send_method;    /* commandSend method */
static MonoMethod *is_started_method;  /* isStarted method */

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
		exit(1);
	}
	
	image = mono_assembly_get_image(assembly);
	klass = mono_class_from_name(image, "IsisService", "IsisServer");
	
	while ((m = mono_class_get_methods(klass, &iter))) {
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
static void* isis_start_fun() {
	void *args[3];
	
	/* Must be called if a new thread is using mono */
	mono_thread_attach(domain);
	
	/* Set up parameters */
	args[0] = &node_num;
	args[1] = &shard_size;
	args[2] = &my_rank;
	
	/* Call createGroup method in C# */
	mono_runtime_invoke(start_method, NULL, args, NULL);
	return NULL;
}

/* Start ISIS */
void isis_start(void) {
	pthread_t thread;
	
	/* Start ISIS */
	pthread_create(&thread, NULL, isis_start_fun, NULL);
}


/* Send command */
int isis_send(char *cmd) {
	MonoString *str;
	void *args[1];
	MonoObject *result;
	
	/* Must be called if a new thread is using mono
	 * Indeed this will be called by another thread
	 */
	mono_thread_attach(domain);
	
	str = mono_string_new(domain, cmd);
	args[0] = str;
	result = mono_runtime_invoke(send_method, NULL, args, NULL);
	
	return *(int*)mono_object_unbox(result);
}

/*
void safe_send(const char *command, int rank) {
}
*/

/*
int main(int argc, char **argv) {
	while (ret == 0) {
	result = mono_runtime_invoke(is_started, NULL, NULL, NULL);
	ret = *(int*)mono_object_unbox(result);
	printf("return value is %d\n", ret);
	sleep(3);
	}
	safe_send("insert li", 10);
}*/
