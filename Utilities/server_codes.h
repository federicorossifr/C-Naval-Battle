typedef enum user_status {
    FREE,BUSY,PLAY_READY,CONNECTING,PLAYING
} user_status;

typedef enum server_response {
    USERINVAL,USEROK, NOUSER, BUSYUSER, CONN_OK, CONN_REJ, MATCH_BEGIN, MATCH_CRASHED
} server_response;
