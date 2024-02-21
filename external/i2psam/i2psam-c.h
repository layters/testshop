/**
 * C wrapper for i2psam
 * Author: jeff
 * License: MIT
 * probably contains bugs :-DDDD
 */

#ifndef I2PSAM_C_H
#define I2PSAM_C_H

#include <stdint.h>
#include <stdlib.h>

struct i2psam_destination;
struct i2psam_stream_session;
struct i2psam_socket;

struct i2psam_stream_settings
{
  /**
   * hostname of sam interface
   */
  const char *samhost;
  /**
   * port of sam interface
   */
  const uint16_t samport;
  /**
   * nickname for sam session
   */
  const char *nickname;
  /**
   * i2cp options string
   */
  const char *i2cp_opts;
  /**
   * destination private key
   */
  const char *destination;
};

/**
 * create new stream session
 */
struct i2psam_stream_session *i2psam_stream_session_new(struct i2psam_stream_settings *);

/**
 * close and free stream session
 */
void i2psam_stream_session_free(struct i2psam_stream_session *);

/**
 * get sam host of stream session
 * @return must be free()'d by caller
 */
const char *i2psam_get_samhost(struct i2psam_stream_session *);

/**
 * get sam port of stream session
 */
uint16_t i2psam_get_samport(struct i2psam_stream_session *);

/**
 * get sam session's nickname
 * @return must be free()'d by caller
 */
const char *i2psam_get_nickname(struct i2psam_stream_session *);

/**
 * get sam session's id
 * @return must be free()'d by caller
 */
const char *i2psam_get_session_id(struct i2psam_stream_session *);

/**
 * get min version from sam session's handshake
 * @return must be free()'d by caller
 */
const char *i2psam_get_sam_min_version(struct i2psam_stream_session *);

/**
 * get max version from sam session's handshake
 * @return must be free()'d by caller
 */
const char *i2psam_get_sam_max_version(struct i2psam_stream_session *);

/**
 * get current version in use with sam session
 * @return must be free()'d by caller
 */
const char *i2psam_get_sam_version(struct i2psam_stream_session *);

/**
 * get i2cp options used by sam session
 * @return must be free()'d by caller
 */
const char *i2psam_get_i2cp_options(struct i2psam_stream_session *);

/**
 * return 1 if session is sick otherwise returns 0
 */
int i2psam_is_sick(struct i2psam_stream_session *);

/**
 * accept a new inbound connection
 * @param silent 0 if we want to obtain the remote's destination, nonzero means don't
 */
struct i2psam_socket *i2psam_accept(struct i2psam_stream_session *, int silent);

/**
 * connect to remote destination
 * @param destination full public destination base64 blob
 * @param silent 0 if we want to get verbose error info from connect, nonzero means don't
 */
struct i2psam_socket *i2psam_connect(struct i2psam_stream_session *, const char *destination, int silent);

/**
 * forward all inbound connections to a remote endpoint
 * @param host remote hostname of endpoint
 * @param port remote port of endpoint
 * @param silent 0 if we want to be verbose when forwarding to remote endpoint, nonzero means don't
 * @return -1 on fail, otherwise 0
 */
int i2psam_forward(struct i2psam_stream_session *, const char *host, uint16_t port, int silent);

/**
 * do a name lookup, if return is non null caller must free()'d
 * @param name the name to resolve
 * @return public destination base64 blob for the name or NULL if the name lookup fails
 */
const char *i2psam_namelookup(struct i2psam_stream_session *, const char *name);

/**
 * generate a new i2p destination keypair, return value must be free()'d when done
 * @return newly generated keypair
 */
struct i2psam_destination *i2psam_dest_generate(struct i2psam_stream_session *);

/**
 * stop forwarding to remote endpoint
 * @param host hostname of remote endpoint
 * @param port port of remote endpoint
 */
void i2psam_stop_forwarding(struct i2psam_stream_session *, const char *host, uint16_t port);

/**
 * stop forwarding to all remote endpoints
 */
void i2psam_stop_forwarding_all(struct i2psam_stream_session *);

/**
 * get remote destination for our stream session
 * @return must be free()'d by caller when done with it
 */
struct i2psam_destination *i2psam_get_my_destination(struct i2psam_stream_session *);

/**
 * blocking write a buffer of data with an i2psocket
 * @param data buffer to be written
 * @param dlen size of buffer
 */
void i2psam_write(struct i2psam_socket *, const char *data, size_t dlen);

/**
 * blocking read on an i2p socket
 * @param pointer to size read
 * @return pointer to read segment, must be free()'d when done if error while reading returns nullptr
 */
char *i2psam_read(struct i2psam_socket *, size_t *dlen);

/**
 * close an i2p socket, does not free()
 */
void i2psam_socket_close(struct i2psam_socket *);

/**
 * @return 1 if an i2p socket is okay otherwise returns 0
 */
int i2psam_socket_is_ok(struct i2psam_socket *);

/**
 * free() an i2p socket, must be closed
 */
void i2psam_socket_free(struct i2psam_socket *);

/**
 * get private key for destination as null terminated base64 string
 * @return must be free()'d by caller when done
 */
const char *i2psam_destination_priv(struct i2psam_destination *);

/**
 * get public base64 destination blob as null terminated string
 * @return must be free()'d by caller when done
 */
const char *i2psam_destination_pub(struct i2psam_destination *);

void i2psam_destination_free(struct i2psam_destination *);

#endif // I2PSAM_C_H
