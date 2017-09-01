#include "ftl.h"
#include "ftl_private.h"
#ifndef DISABLE_AUTO_INGEST
#include <curl/curl.h>
#include <jansson.h>
#endif

static int _ingest_lookup_ip(const char *ingest_location, char ***ingest_ip);
static int _ping_server(const char *ip, int port);
OS_THREAD_ROUTINE _ingest_get_rtt(void *data);

typedef struct {
    ftl_ingest_t *ingest;
    ftl_stream_configuration_private_t *ftl;
}_tmp_ingest_thread_data_t;

static int _ping_server(const char *ip, int port) {

  SOCKET sock;
  struct sockaddr_in server_addr;
  unsigned char buf[sizeof(struct in_addr)];
  uint8_t dummy[4];
  struct timeval start, stop, delta;
  int retval = -1;

  do {
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
    {
      break;
    }

    if (inet_pton(AF_INET, ip, buf) == 0) {
      break;
    }

    //Prepare the sockaddr_in structure
    server_addr.sin_family = AF_INET;
    memcpy((char *)&server_addr.sin_addr.s_addr, (char *)buf, sizeof(buf));
    server_addr.sin_port = htons(port);

    set_socket_recv_timeout(sock, 500);

    gettimeofday(&start, NULL);

    if (sendto(sock, dummy, sizeof(dummy), 0, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in)) == SOCKET_ERROR) {
      break;
    }

    if (recv(sock, dummy, sizeof(dummy), 0) < 0) {
      break;
    }

    gettimeofday(&stop, NULL);
    timeval_subtract(&delta, &stop, &start);
    retval = (int)timeval_to_ms(&delta);
  } while (0);

  shutdown_socket(sock, SD_BOTH);
  close_socket(sock);

  return retval;
}

OS_THREAD_ROUTINE _ingest_get_rtt(void *data) {
    _tmp_ingest_thread_data_t *thread_data = (_tmp_ingest_thread_data_t *)data;
    ftl_stream_configuration_private_t *ftl = thread_data->ftl;
    ftl_ingest_t *ingest = thread_data->ingest;
    int ping;

    ingest->rtt = 1000;

    if ((ping = _ping_server(ingest->ip, INGEST_PING_PORT)) >= 0) {
        ingest->rtt = ping;
    }

    return 0;
}

ftl_status_t find_closest_available_ingest(const char* ingestIps[], int ingestsCount, char* bestIngestIpComputed)
{
    ftl_ingest_t* ingestElements;
    int i;

    if ((ingestElements = malloc(sizeof(ftl_ingest_t) * ingestsCount)) == NULL) {
        return FTL_MALLOC_FAILURE;
    }

    for (i =0; i < ingestsCount; i++) {
        strcpy_s(ingestElements[i].ip, sizeof(ingestElements[i].ip), ingestIps[i]);
        ingestElements[i].rtt = 1000;
        ingestElements[i].next = NULL;
    }

    OS_THREAD_HANDLE *handles;
    _tmp_ingest_thread_data_t *data;

    ftl_ingest_t *elmt, *best = NULL;
    struct timeval start, stop, delta;

    if ((handles = (OS_THREAD_HANDLE *)malloc(sizeof(OS_THREAD_HANDLE) * ingestsCount)) == NULL) {
        free(ingestElements);
        return FTL_MALLOC_FAILURE;
    }

    if ((data = (_tmp_ingest_thread_data_t *)malloc(sizeof(_tmp_ingest_thread_data_t) * ingestsCount)) == NULL) {
        free(ingestElements);
        free(handles);
        return FTL_MALLOC_FAILURE;
    }

    gettimeofday(&start, NULL);

    /*query all the ingests about cpu and rtt*/
    for (i = 0; i < ingestsCount; i++) {
        handles[i] = 0;
        data[i].ingest = &ingestElements[i];
        data[i].ftl = NULL;
        os_create_thread(&handles[i], NULL, _ingest_get_rtt, &data[i]);
        sleep_ms(5); //space out the pings
    }

    /*wait for all the ingests to complete*/
    for (i = 0; i < ingestsCount; i++) {

        if (handles[i] != 0) {
            os_wait_thread(handles[i]);
        }

        if (best == NULL || ingestElements[i].rtt < best->rtt) {
            best = &ingestElements[i];
        }
    }

    gettimeofday(&stop, NULL);
    timeval_subtract(&delta, &stop, &start);
    int ms = (int)timeval_to_ms(&delta);

    for (i = 0; i < ingestsCount; i++) {
        if (handles[i] != 0) {
            os_destroy_thread(handles[i]);
        }
    }

    free(handles);
    free(data);

    if (best) {
        strcpy_s(bestIngestIpComputed, sizeof(best->ip), best->ip);
        free(ingestElements);
        return FTL_SUCCESS;
    }

    free(ingestElements);

    return FTL_UNKNOWN_ERROR_CODE;
}

#ifndef DISABLE_AUTO_INGEST
OS_THREAD_ROUTINE _ingest_get_hosts(ftl_stream_configuration_private_t *ftl);

static size_t _curl_write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if (mem->memory == NULL) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

OS_THREAD_ROUTINE _ingest_get_hosts(ftl_stream_configuration_private_t *ftl) {
  CURL *curl_handle;
  CURLcode res;
  struct MemoryStruct chunk;
  char *query_result = NULL;
  size_t i = 0;
  int total_ingest_cnt = 0;
  json_error_t error;
  json_t *ingests = NULL, *ingest_item = NULL;

  curl_handle = curl_easy_init();

  chunk.memory = malloc(1);  /* will be grown as needed by realloc */
  chunk.size = 0;    /* no data at this point */

  curl_easy_setopt(curl_handle, CURLOPT_URL, INGEST_LIST_URI);
  curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, TRUE);
  curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 2L);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, _curl_write_callback);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "ftlsdk/1.0");

#if LIBCURL_VERSION_NUM >= 0x072400
  // A lot of servers don't yet support ALPN
  curl_easy_setopt(curl_handle, CURLOPT_SSL_ENABLE_ALPN, 0);
#endif

  res = curl_easy_perform(curl_handle);

  if (res != CURLE_OK) {
    printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    goto cleanup;
  }

  if ((ingests = json_loadb(chunk.memory, chunk.size, 0, &error)) == NULL) {
    goto cleanup;
  }
  
  size_t size = json_array_size(ingests);

  for (i = 0; i < size; i++) {
    const char *name, *ip;
    ingest_item = json_array_get(ingests, i);
    json_unpack(ingest_item, "{s:s, s:s}", "name", &name, "ip", &ip);

    ftl_ingest_t *ingest_elmt;

    if ((ingest_elmt = malloc(sizeof(ftl_ingest_t))) == NULL) {
      goto cleanup;
    }

    strcpy_s(ingest_elmt->name, sizeof(ingest_elmt->name), name);
    strcpy_s(ingest_elmt->ip, sizeof(ingest_elmt->ip), ip);
    ingest_elmt->rtt = 500;

    ingest_elmt->next = NULL;

    if (ftl->ingest_list == NULL) {
      ftl->ingest_list = ingest_elmt;
    }
    else {
      ftl_ingest_t *tail = ftl->ingest_list;
      while (tail->next != NULL) {
        tail = tail->next;
      }

      tail->next = ingest_elmt;
    }

    total_ingest_cnt++;
  }

cleanup:
  free(chunk.memory);
  curl_easy_cleanup(curl_handle);
  if (ingests != NULL) {
    json_decref(ingests);
  }

  ftl->ingest_count = total_ingest_cnt;

  return total_ingest_cnt;
}

char * ingest_find_best(ftl_stream_configuration_private_t *ftl) {

  OS_THREAD_HANDLE *handle;
  _tmp_ingest_thread_data_t *data;
  int i;
  ftl_ingest_t *elmt, *best = NULL;
  struct timeval start, stop, delta;

  /*get list of ingest each time as they are dynamically selected*/
  while (ftl->ingest_list != NULL) {
    elmt = ftl->ingest_list;
    ftl->ingest_list = elmt->next;
    free(elmt);
  }

  if (_ingest_get_hosts(ftl) <= 0) {
    return NULL;
  }

  if ((handle = (OS_THREAD_HANDLE *)malloc(sizeof(OS_THREAD_HANDLE) * ftl->ingest_count)) == NULL) {
    return NULL;
  }

  if ((data = (_tmp_ingest_thread_data_t *)malloc(sizeof(_tmp_ingest_thread_data_t) * ftl->ingest_count)) == NULL) {
    return NULL;
  }

  gettimeofday(&start, NULL);

  /*query all the ingests about cpu and rtt*/
  elmt = ftl->ingest_list;
  for (i = 0; i < ftl->ingest_count && elmt != NULL; i++) {
    handle[i] = 0;
    data[i].ingest = elmt;
    data[i].ftl = ftl;
    os_create_thread(&handle[i], NULL, _ingest_get_rtt, &data[i]);
    sleep_ms(5); //space out the pings
    elmt = elmt->next;
  }

  /*wait for all the ingests to complete*/
  elmt = ftl->ingest_list;
  for (i = 0; i < ftl->ingest_count && elmt != NULL; i++) {

    if (handle[i] != 0) {
      os_wait_thread(handle[i]);
    }

    if (best == NULL || elmt->rtt < best->rtt) {
      best = elmt;
    }

    elmt = elmt->next;
  }

  gettimeofday(&stop, NULL);
  timeval_subtract(&delta, &stop, &start);
  int ms = (int)timeval_to_ms(&delta);

  FTL_LOG(ftl, FTL_LOG_INFO, "It took %d ms to query all ingests\n", ms);

  elmt = ftl->ingest_list;
  for (i = 0; i < ftl->ingest_count && elmt != NULL; i++) {
    if (handle[i] != 0) {
      os_destroy_thread(handle[i]);
    }

    elmt = elmt->next;
  }

  free(handle);
  free(data);

  if (best){
    FTL_LOG(ftl, FTL_LOG_INFO, "%s at ip %s had the shortest RTT of %d ms\n", best->name, best->ip, best->rtt);
    return _strdup(best->ip);
  }

  return NULL;
}
#endif

void ingest_release(ftl_stream_configuration_private_t *ftl) {

  ftl_ingest_t *elmt, *tmp;

  elmt = ftl->ingest_list;

  while (elmt != NULL) {
    tmp = elmt->next;
    free(elmt);
    elmt = tmp;
  }
}

char * ingest_get_ip(ftl_stream_configuration_private_t *ftl, char *host) {
  int total_ips;
  char **ips;
  char *ip;
  int i;

  if ((total_ips = _ingest_lookup_ip(host, &ips)) <= 0) {
    return NULL;
  }

  ip = _strdup(ips[0]);

  for (i = 0; i < total_ips; i++) {
    free(ips[i]);
  }

  free(ips);

  return ip;
}

static int _ingest_lookup_ip(const char *ingest_location, char ***ingest_ip) {
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  struct sockaddr_in  *sockaddr_ipv4;
  int ips_found = 0;
  int s;
  BOOL success = FALSE;
  ingest_ip[0] = '\0';
  char ip[INET_ADDRSTRLEN];

  if (*ingest_ip != NULL) {
    return -1;
  }

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;    
  hints.ai_socktype = 0;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;

  if ((s = getaddrinfo(ingest_location, NULL, &hints, &result)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    return 0;
  }

  int total_ips = 0;
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    total_ips++;
  }

  if ((*ingest_ip = malloc(sizeof(char*) * total_ips)) == NULL) {
    return 0;
  }
  
  ips_found = 0;
  for (rp = result; rp != NULL; rp = rp->ai_next, ips_found++) {
    if (((*ingest_ip)[ips_found] = malloc(IPV4_ADDR_ASCII_LEN)) == NULL) {
      return 0;
    }

    sockaddr_ipv4 = (struct sockaddr_in *) rp->ai_addr;

    if (inet_ntop(AF_INET, &sockaddr_ipv4->sin_addr, ip, sizeof(ip)) == NULL) {
      continue;
    }

    strcpy_s((*ingest_ip)[ips_found], IPV4_ADDR_ASCII_LEN, ip);
  }

  freeaddrinfo(result);

  return ips_found;
}
