/* -*- linux-c -*- */
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <cyc.h>

void test_libcyc_ll(struct cyc_info *);
void test_stack(void);
void test_socket(struct cyc_info *);
void test_shmem(struct cyc_info *);

void test_shmem(struct cyc_info *cyc)
{
	char *buf = NULL;
	key_t ipc_key = ERROR;
	int shmid = ERROR;

	printf("%s entering\n", __func__);

	ipc_key = libcyc_ipc_key_get_private();
	if ((shmid = libcyc_ipc_shmem_create(cyc, ipc_key, 1024)) == ERROR) {
		printf("Shared memory create failed\n");
		return;
	}
	buf = (char *)libcyc_ipc_shmem_attach(cyc, shmid, NULL, 0);
	strncpy(buf, "hello world", MAX_STR_LEN);
	printf("printing string from shared memory: %s\n", buf);
	libcyc_ipc_shmem_detach(cyc, shmid);
	libcyc_ipc_shmem_destroy(cyc, 0);
}

void test_sem(struct cyc_info *cyc)
{
	key_t ipc_key = ERROR;
	int semid = ERROR, i;
	int nsems = 0;
	int semval = ERROR;

	printf("%s entering\n", __func__);

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT)))
		printf("libcyc not initialized\n");
	
	ipc_key = libcyc_ipc_key_get_private();
	semid = libcyc_ipc_sem_create(cyc, ipc_key, 5);
	nsems = libcyc_ipc_sem_nsems_get(cyc, semid);
	printf("Semaphore set created with %d sems\n", nsems);
	printf("Setting all semaphores to value 1\n");
	libcyc_ipc_sem_set_all(cyc, semid, 1);
	for (i = 0; i < nsems; i++) {
		if ((semval = libcyc_ipc_sem_getval(cyc, semid, i)) != 1)
			printf("Invalid value on semaphore: %d val: %d\n",
			       i, semval);
	}
	printf("Setting all semaphores to value 0\n");
	libcyc_ipc_sem_set_all(cyc, semid, 0);
	for (i = 0; i < nsems; i++) {
		if ((semval = libcyc_ipc_sem_getval(cyc, semid, i)) != 0)
			printf("Invalid value on semaphore: %d val: %d\n",
			       i, semval);
	}
	libcyc_ipc_sem_op(cyc, semid, 0, 1);
	libcyc_ipc_sem_op(cyc, semid, 0, 1);
	if ((semval = libcyc_ipc_sem_getval(cyc, semid, 0))) {
		printf("Semaphore count incremented %d\n", semval);
	} else {
		printf("Error increasing semaphore value\n");
	}
	libcyc_ipc_sem_op(cyc, semid, 0, -1);
	libcyc_ipc_sem_op(cyc, semid, 0, -1);
	if ((semval = libcyc_ipc_sem_getval(cyc, semid, 0)) == 0) {
		printf("Semaphore count decremented %d\n", semval);
	} else {
		printf("Error decrementing semaphore value\n");
	}
	libcyc_ipc_sem_destroy(cyc, semid);
	printf("Semaphore set destroyed\n");

	printf("Timed semaphore (5 secs)\n");
	ipc_key = libcyc_ipc_key_get_private();
        semid = libcyc_ipc_sem_create(cyc, ipc_key, 1);
	libcyc_ipc_sem_set_all(cyc, semid, 0);	
	libcyc_ipc_sem_timed_op(cyc, semid, 0, 1, 5000);
	if (libcyc_ipc_sem_getval(cyc, semid, 0) != 1) {
		printf("Error incrementing semaphore value to 1\n");
	}
	if (libcyc_ipc_sem_timed_op(cyc, semid, 0, -1, 5000) == -1) {
		printf("Error decrementing semaphore value to 0\n");
	}
	if (libcyc_ipc_sem_timed_op(cyc, semid, 0, -1, 5000) == -1) {
		printf("Semaphore timed operation completed successfully\n");
	}
	libcyc_ipc_sem_destroy(cyc, semid);
	printf("Timed semaphore test complete\n");
}

void test_mutex(struct cyc_info *cyc)
{
	pthread_mutex_t *mutex;
	int ret = 0;

	printf("%s entering\n", __func__);

	mutex = libcyc_mutex_create(cyc);
	printf("Mutex created\n");
	printf("libcyc_mutex_lock: %d\n", libcyc_mutex_lock(cyc, mutex));
	if ((ret = libcyc_mutex_trylock(cyc, mutex))) {
		printf("Testing lock: Mutex locked!\n");
	} else {
		printf("Testing lock: An error occurred locking the mutex! ret: %d\n", ret);
	}
	printf("libcyc_mutex_unlock: %d\n", libcyc_mutex_unlock(cyc, mutex));
	if ((ret = libcyc_mutex_trylock(cyc, mutex))) {
		printf("Testing lock: An error occurred unlocking the mutex! ret: %d\n", ret);
	} else {
		printf("Testnig lock: Mutex unlocked!\n");
	}
	libcyc_mutex_destroy(cyc, NULL);
}

void test_socket(struct cyc_info *cyc)
{
	int sock;
	char *tmp;

	sock=libcyc_sock_init(cyc);
	libcyc_sock_send(cyc,sock,"hello world");
	tmp=libcyc_sock_recv(cyc,sock);

	printf("got: %s\n",tmp);

	libcyc_sock_destroy(cyc,sock);
}

void test_stack(void)
{
	libcyc_list *stack=NULL;
	char *hello="hello";
	char *world="world";
	char *tmp1,*tmp2,*tmp3;

	libcyc_push(&stack,world);
	libcyc_push(&stack,hello);

	tmp1=(char *)libcyc_pop(&stack);
	tmp2=(char *)libcyc_pop(&stack);
	tmp3=(char *)libcyc_pop(&stack);

	printf("popped values: %s %s %s\n",tmp1,tmp2,tmp3);
}

void test_kvm(struct cyc_info *cyc)
{
	int teststat=0;

	if (cyc == NULL) return;

	printf("%s entering\n",__func__);

	libcyc_kvm_init(cyc);
	printf("KVM ports: %d\n",libcyc_kvm_get_ports(cyc));
	printf("KVM model: %d\n",libcyc_kvm_get_model(cyc));
	printf("KVM IP module map: %d\n",libcyc_kvm_get_ip_modules_map(cyc));
	printf("Number of IP modules: %d\n",libcyc_kvm_get_num_ip_modules(cyc));

	/* Test the status LEDs */
	if (libcyc_kvm_get_model(cyc) == MODEL_KVMNET_PLUS) {
		int ledstat=ERROR;
		printf("%s LED Status: 0x%x\n",
		       __func__, libcyc_kvm_led_get_status(cyc));

		printf("%s ALL LEDs OFF\n", __func__);
		libcyc_kvm_led_set_off(cyc, CARD_1|CARD_2|CARD_3|CARD_4);
		if ((ledstat=libcyc_kvm_led_get_status(cyc) != CYC_CLEAR)) teststat++;

		sleep(1);

		libcyc_kvm_led_set(cyc, CARD_1);
		usleep(80000);
		libcyc_kvm_led_set(cyc, CARD_2);
		usleep(80000);
		libcyc_kvm_led_set(cyc, CARD_3);
		usleep(80000);
		libcyc_kvm_led_set(cyc, CARD_4);
		usleep(80000);
		libcyc_kvm_led_set(cyc, CARD_3);
		usleep(80000);
		libcyc_kvm_led_set(cyc, CARD_2);
		usleep(80000);
		libcyc_kvm_led_set(cyc, CARD_1);
		usleep(80000);

		printf("%s ALL LEDs ON\n",__func__);
		libcyc_kvm_led_set_on(cyc, CARD_1|CARD_2|CARD_3|CARD_4);
		if ((ledstat=libcyc_kvm_led_get_status(cyc) != 0xF)) teststat++;

		sleep(1);

		libcyc_kvm_led_set(cyc, libcyc_kvm_get_ip_modules_map(cyc));
	}

	if (teststat) {
		printf("At least one of the KVM tests failed!\n");
	}
}

void test_apctl(struct cyc_info *cyc)
{
	int teststat = 0;

        printf("Fan #1 Speed: %d rpm\n",
               libcyc_apctl_get_fan_speed(cyc,FAN_1));
        printf("Fan #2 Speed: %d rpm\n",
               libcyc_apctl_get_fan_speed(cyc,FAN_2));
        printf("Fan #3 Speed: %d rpm\n",
               libcyc_apctl_get_fan_speed(cyc,FAN_3));

	printf("Alarm on\n");
	libcyc_apctl_set_alarm(cyc, ON);
	if (libcyc_apctl_get_alarm(cyc) == OFF) teststat++;
	printf("Alarm off\n");
	libcyc_apctl_set_alarm(cyc, OFF);
	if (libcyc_apctl_get_alarm(cyc) == ON) teststat++;

	/* Test the GP/HD LED */
	printf("GP LED on\n");
	libcyc_apctl_set_gpled(cyc, ON);
	if (libcyc_apctl_get_gpled(cyc) == OFF) teststat++;
	sleep(1);
	printf("GP LED off\n");
	libcyc_apctl_set_gpled(cyc, OFF);
	if (libcyc_apctl_get_gpled(cyc) == ON) teststat++;

	if (libcyc_kvm_get_model(cyc) != MODEL_KVMNET_PLUS) {
		printf("HD LED on\n");
		libcyc_apctl_set_hdleden(cyc, ON);
		if (libcyc_apctl_get_hdleden(cyc) == OFF) teststat++;
		sleep(1);
		printf("HD LED off\n");
		libcyc_apctl_set_hdleden(cyc, OFF);
		if (libcyc_apctl_get_hdleden(cyc) == ON) teststat++;
	}
		
	if (teststat) {
                printf("At least one of the apctl tests failed!\n");
        }
}

void test_get_ip(struct cyc_info *cyc)
{
	char *ipaddr_str = libcyc_iface_get_ip(cyc, "eth0");

	printf("%s IP address: %s\n",__func__, ipaddr_str);

	free(ipaddr_str);
	ipaddr_str = NULL;
}

void test_libcyc_ll(struct cyc_info *cyc)
{
	libcyc_list *list=NULL,*list2=NULL,*list3=NULL;
	char *tmp_str;
	char *str=libcyc_malloc(cyc,10);
	char *str1=libcyc_malloc(cyc,10);
	char *str2=libcyc_malloc(cyc,10);

	if (cyc == NULL) return;

	strcpy(str,"hello");
	strcpy(str1,"world");
	strcpy(str2,"again");

	printf("Print strings with mem from libcyc_malloc\n");
	printf("strings: %s %s %s\n",str,str1,str2);
	printf("ptr vals: %d %d %d\n",(int)str,(int)str1,(int)str2);

	printf("Using libcyc_ll_append to assign strings to a ll\n");
	list=libcyc_ll_append(list,str);
	list=libcyc_ll_append(list,str1);
	list=libcyc_ll_append(list,str2);

	printf("Iterating over the list\n");
	while ((tmp_str=libcyc_ll_iterate(list))) {
		printf("Iterating: %s\n",tmp_str);
	}

	printf("Removing a value from the list\n");
	list=libcyc_ll_remove(list,str);

	printf("Iterating over the list again\n");
	while ((tmp_str=libcyc_ll_iterate(list))) {
		printf("Iterating again: %s\n",tmp_str);
	}

	printf("Freeing the list\n");
	libcyc_ll_free(list);
	list=NULL;

	printf("Create two one-item lists\n");
	list2=libcyc_ll_append(list2,str);
	list3=libcyc_ll_append(list3,str);
	printf("Iterating one-item list:\n");
	while ((tmp_str=libcyc_ll_iterate(list2))) {
		printf("Iterating: %s\n",tmp_str);
	}

	printf("Iterate over the one-item list again\n");
	while ((tmp_str=libcyc_ll_iterate(list2))) {
		printf("Iterating: %s\n",tmp_str);
	}

	printf("Remove the only item from a one-item list\n");
	list2=libcyc_ll_remove(list2,str);

	printf("Free all memory used; free lists\n");
	libcyc_free_all(cyc);
	printf("Freeing list2\n");
	libcyc_ll_free(list2);
	list2=NULL;
	printf("Freeing list3\n");
	libcyc_ll_free(list3);
	list3=NULL;

	printf("Done with %s\n",__func__);
}

void libcyc_ll_fifo(struct cyc_info *cyc)
{
	libcyc_list *fifo = NULL;
	char *str = "hello world";
	char *str1 = "hello again";
	char *str2 = "testing";
	char *tmp = NULL;

	/* Put data into the FIFO */
	fifo = libcyc_ll_append(fifo, str);
	fifo = libcyc_ll_append(fifo, str1);
	fifo = libcyc_ll_append(fifo, str2);

	/* Take the data out in FIFO order */
	while((tmp = (char *)libcyc_ll_iterate(fifo))) {
		fifo = libcyc_ll_remove(fifo, tmp);
		printf("data: %s\n", tmp);
	}
	
	libcyc_ll_free(fifo);
	fifo = NULL;
}

int main(int argc,char **argv) {
	struct cyc_info *cyc1=NULL,*cyc2=NULL,*cyc3=NULL,*cyc4=NULL;

	if ((cyc1=libcyc_init(CYC_USE_STDERR|CYC_DAEMON|CYC_DEBUG,
			      "test-daemon","1.0","/tmp","/tmp")) == NULL) {
		return(ERROR);
	}
	if ((cyc2=libcyc_init(CYC_USE_STDERR|CYC_CGI|CYC_DEBUG,"test","1.0","/tmp","/tmp")) == NULL) {
		libcyc_destroy(cyc1);
		return(ERROR);
	}
	if ((cyc3=libcyc_init(CYC_DEBUG,"test-other","1.0","/tmp","/tmp")) == NULL) {
		libcyc_destroy(cyc1);
		libcyc_destroy(cyc2);
		return(ERROR);
	}
	if ((cyc4=libcyc_init(CYC_DEBUG,"test-other","1.0","/tmp","/tmp")) == NULL) {
		printf("Library locking works!\n");
	}

	printf("Begin tests of libcyc library functions:\n\n");

	printf("Test the shared memory functions\n");
	test_shmem(cyc1);	
	printf("Test the semaphore functions\n");
	test_sem(cyc1);
	printf("Test mutex locking\n");
	test_mutex(cyc1);

	printf("Test the ll and malloc functions\n");	
	test_libcyc_ll(cyc1);
	test_libcyc_ll(cyc2);
	test_libcyc_ll(cyc3);

	printf("Test the stack functions\n");
	test_stack();

	printf("Test the socket functions\n");
	test_socket(cyc1);
	test_socket(cyc2);
	test_socket(cyc3);

	printf("Test the kvm functions\n");
	test_kvm(cyc1);
	printf("Test the apctl functions\n");
	test_apctl(cyc1);
	printf("Test the get_ip utility function\n");
	test_get_ip(cyc1);

	libcyc_ll_fifo(cyc1);

	libcyc_destroy(cyc1);
	libcyc_destroy(cyc2);
	libcyc_destroy(cyc3);
	libcyc_destroy(cyc4);

	return(0);
}
