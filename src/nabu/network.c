/*
 * NABU network layer -- HTTP via the NIA file store, AppKey storage
 * as local DAT files, and session ID management.
 */

#include "../misc.h"
#include "../../../../NABULIB/RetroNET-FileStore.h"

/* lobby/game handoff: PLAYER= and URL= written by NLOBBY, read here */
#define LOBBYSEL_FILE "LOBBYSEL.DAT"
#define SESSION_FILE "NBSESSID.DAT"

extern ClientState clientState;

static uint8_t io_buf[256];
static char line_buf[96];
static uint16_t request_counter;
static uint16_t session_id;
static bool session_ready;

static void ensure_session_id(void);

/* Trim trailing newlines. */
static void trim_newline(char *s)
{
    uint8_t i = 0;

    while (s[i]) {
        if (s[i] == '\r' || s[i] == '\n') {
            s[i] = 0;
            return;
        }
        i++;
    }
}

/* Copy bounded text. */
static void copy_text(char *dst, const char *src, uint8_t max_len)
{
    uint8_t i = 0;

    while (src[i] && i + 1 < max_len) {
        dst[i] = src[i];
        i++;
    }

    dst[i] = 0;
}

/* Append bounded text. */
static void append_text(char *dst, const char *src, uint8_t max_len)
{
    uint8_t pos = (uint8_t)strlen(dst);
    uint8_t i = 0;

    while (src[i] && pos + 1 < max_len) {
        dst[pos++] = src[i++];
    }

    dst[pos] = 0;
}

/* cache-fix unique URL -- session & request count -- 1.00.57 */
static void build_cache_busted_url(const char *plain_url, char *final_url, uint8_t final_len)
{
    copy_text(final_url, plain_url, final_len);

    if (strstr(final_url, "http://") == final_url || strstr(final_url, "https://") == final_url) {
        ensure_session_id();
        append_text(final_url, strchr(final_url, '?') ? "&_s=" : "?_s=", final_len);
        sprintf(line_buf, "%u", (unsigned)session_id);
        append_text(final_url, line_buf, final_len);
        append_text(final_url, "&_n=", final_len);
        sprintf(line_buf, "%u", (unsigned)request_counter++);
        append_text(final_url, line_buf, final_len);
    }
}

/* session counter, NBSESSID.DAT -- 1.00.57 (nonce) */
static void ensure_session_id(void)
{
    uint8_t fh;
    uint16_t count = 0;

    if (session_ready)
        return;

    fh = rn_fileOpen((uint8_t)strlen(SESSION_FILE),
                     (uint8_t *)SESSION_FILE,
                     OPEN_FILE_FLAG_READWRITE,
                     0xFF);
    if (fh != 0xFF) {
        count = rn_fileHandleReadSeq(fh, io_buf, 0, 8);
        if (count > 0) {
            io_buf[count] = 0;
            session_id = (uint16_t)atoi((char *)io_buf);
        }
    } else {
        fh = rn_fileOpen((uint8_t)strlen(SESSION_FILE),
                         (uint8_t *)SESSION_FILE,
                         OPEN_FILE_FLAG_READWRITE,
                         0xFF);
    }

    session_id++;
    if (session_id == 0)
        session_id = 1;

    if (fh != 0xFF) {
        rn_fileHandleEmptyFile(fh);
        sprintf(line_buf, "%u", (unsigned)session_id);
        rn_fileHandleAppend(fh, 0, (uint16_t)strlen(line_buf), (uint8_t *)line_buf);
        rn_fileHandleClose(fh);
    }

    session_ready = true;
}

/* Return the current session id. */
uint16_t nabu_get_session_id(void)
{
    ensure_session_id();
    return session_id;
}

/* Load a small file into memory. */
static uint16_t load_file_bytes(const char *name, uint8_t *buffer, uint16_t max_len)
{
    uint8_t fh;
    uint16_t got;
    uint16_t pos = 0;

    fh = rn_fileOpen((uint8_t)strlen(name), (uint8_t *)name, OPEN_FILE_FLAG_READONLY, 0xFF);
    if (fh == 0xFF) {
        return 0;
    }

    while (pos < max_len) {
        uint16_t want = (uint16_t)(max_len - pos);
        if (want > 64) want = 64;
        got = rn_fileHandleReadSeq(fh, buffer, pos, want);
        if (got == 0)
            break;
        pos = (uint16_t)(pos + got);
    }

    rn_fileHandleClose(fh);
    return pos;
}

/* Find a key value in a text buffer. */
static bool find_key_value(const uint8_t *buffer, uint16_t size, const char *key, char *value, uint8_t value_len)
{
    uint16_t i = 0;
    uint8_t line_len = 0;
    uint8_t key_len = (uint8_t)strlen(key);

    while (i < size) {
        char c = (char)buffer[i++];
        if (c == '\r')
            continue;

        if (c == '\n') {
            line_buf[line_len] = 0;
            if (strncmp(line_buf, key, key_len) == 0 && line_buf[key_len] == '=') {
                copy_text(value, line_buf + key_len + 1, value_len);
                trim_newline(value);
                return true;
            }
            line_len = 0;
        } else if (line_len + 1 < sizeof(line_buf)) {
            line_buf[line_len++] = c;
        }
    }

    if (line_len > 0) {
        line_buf[line_len] = 0;
        if (strncmp(line_buf, key, key_len) == 0 && line_buf[key_len] == '=') {
            copy_text(value, line_buf + key_len + 1, value_len);
            trim_newline(value);
            return true;
        }
    }

    return false;
}

/* shared code prefixes URLs with n: -- NABU does not use that device prefix */
/* Remove the network prefix. */
static void strip_network_prefix(const char *url, char *plain_url, uint8_t plain_len)
{
    if (url[0] == 'n' && url[1] == ':')
        copy_text(plain_url, url + 2, plain_len);
    else if (url[0] == 'N' && url[1] == ':')
        copy_text(plain_url, url + 2, plain_len);
    else
        copy_text(plain_url, url, plain_len);
}

/* Perform a RetroNET fetch. */
int16_t custom_network_call(char *url, uint8_t *buffer, uint16_t max_len)
{
    uint8_t fh;
    uint16_t got;
    uint16_t pos = 0;
    static char plain_url[160];
    static char final_url[176];

    strip_network_prefix(url, plain_url, sizeof(plain_url));
    build_cache_busted_url(plain_url, final_url, sizeof(final_url));

    fh = rn_fileOpen((uint8_t)strlen(final_url), (uint8_t *)final_url, OPEN_FILE_FLAG_READONLY, 0xFF);
    if (fh == 0xFF) {
        return 0;
    }

    while (pos < max_len) {
        uint16_t want = (uint16_t)(max_len - pos);
        if (want > 64) want = 64;
        got = rn_fileHandleReadSeq(fh, buffer, pos, want);
        if (got == 0)
            break;
        pos = (uint16_t)(pos + got);
    }

    rn_fileHandleClose(fh);
    return (int16_t)pos;
}

/* Read one local appkey value. */
uint16_t custom_read_appkey(uint16_t creator_id, uint8_t app_id, uint8_t key_id, char *destination)
{
    uint16_t size;

    destination[0] = 0;

    if (creator_id == AK_LOBBY_CREATOR_ID && app_id == AK_LOBBY_APP_ID) {
        size = load_file_bytes(LOBBYSEL_FILE, io_buf, sizeof(io_buf));
        if (size == 0)
            return 0;

        if (key_id == AK_LOBBY_KEY_USERNAME) {
            if (find_key_value(io_buf, size, "PLAYER", destination, 32)) {
                return (uint16_t)strlen(destination);
            }
        } else if (key_id == AK_LOBBY_KEY_SERVER) {
            if (find_key_value(io_buf, size, "URL", destination, 64)) {
                return (uint16_t)strlen(destination);
            }
        }

        return 0;
    }

    return 0;
}

/* Write one local appkey value. */
void custom_write_appkey(uint16_t creator_id, uint8_t app_id, uint8_t key_id, uint16_t count, char *data)
{
    (void)creator_id; (void)app_id; (void)key_id; (void)count; (void)data;
}

/* Log client state. */
void nabu_log_client_state(const char *label)
{
    label = label;
}
