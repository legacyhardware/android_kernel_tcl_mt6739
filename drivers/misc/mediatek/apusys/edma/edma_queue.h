
#ifndef __EDMA_QUEUE_H__
#define __EDMA_QUEUE_H__

#define DECLARE_VLIST(type) \
struct type ## _list { \
	struct type node; \
	struct list_head link; \
}

#define vlist_node_of(ptr, type) ({ \
	const struct list_head *mptr = (ptr); \
	(type *)((char *)mptr - offsetof(type ## _list, link)); })

#define vlist_link(ptr, type) \
	((struct list_head *)((char *)ptr + offsetof(type ## _list, link)))

#define vlist_type(type) type ## _list

#define vlist_node(ptr, type) ((type *)ptr)

enum edma_command_type {
	EDMA_PROC_NORMAL,
	EDMA_PROC_FILL,
	EDMA_PROC_NUMERICAL,
	EDMA_PROC_FORMAT,
	EDMA_PROC_COMPRESS,
	EDMA_PROC_DECOMPRESS,
	EDMA_PROC_RAW,
	EDMA_PROC_EXT_MODE,
	EDMA_PROC_MAX,
};


int edma_alloc_request(struct edma_request **rreq);
int edma_free_request(struct edma_request *req);

int edma_create_user(struct edma_user **user, struct edma_device *edma_device);
int edma_delete_user(struct edma_user *user, struct edma_device *edma_device);

int edma_enque_routine_loop(void *arg);
int edma_flush_requests_from_queue(struct edma_user *user);
int edma_free_request(struct edma_request *req);

void edma_setup_normal_request(struct edma_request *req,
			       struct edma_normal *edma_normal,
			       unsigned int type);
void edma_setup_fill_request(struct edma_request *req,
			       struct edma_fill *edma_fill,
			       unsigned int type);
void edma_setup_ext_mode_request(struct edma_request *req,
			       struct edma_ext *edma_ext,
			       unsigned int type);
void edma_setup_numerical_request(struct edma_request *req,
			       struct edma_numerical *edma_numerical,
			       unsigned int type);
void edma_setup_format_request(struct edma_request *req,
			       struct edma_format *edma_format,
			       unsigned int type);
void edma_setup_compress_request(struct edma_request *req,
			       struct edma_compress *edma_compress,
			       unsigned int type);
void edma_setup_decompress_request(struct edma_request *req,
			       struct edma_decompress *edma_decompress,
			       unsigned int type);

void edma_setup_raw_request(struct edma_request *req,
			       struct edma_raw *edma_raw,
			       unsigned int type);
int edma_push_request_to_queue(struct edma_user *user,
			      struct edma_request *req);
int edma_pop_request_from_queue(u64 handle,
			       struct edma_user *user,
			       struct edma_request **rreq);

#endif /* __EDMA_QUEUE_H__ */
