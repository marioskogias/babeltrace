#ifndef __SCRIBE_C_H__
#define __SCRIBE_C_H__

#ifdef __cplusplus
extern "C" {
#endif

struct scribe_client;

struct scribe_client * open_connection(const char *host, const int port);
int scribe_log(struct scribe_client * c, const char *category, const char *buf);
int close_connection(struct scribe_client *c);

#ifdef __cplusplus
}
#endif

#endif

