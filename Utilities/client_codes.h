typedef enum client_request {
	LOG_IN,WHO,CONN_REQ,QUIT,CONN_ACK,CONN_NAK,READY,MATCH_END
} client_request;


typedef enum in_game_message {
	FIRE,SURR,SHIP_HIT,SHIP_MISSED,YOU_WIN
} in_game_message;