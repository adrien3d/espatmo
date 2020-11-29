#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "sdkconfig.h"

#ifndef espatmo_h
#define espatmo_h

const char* URL_AUTH_REQ = "https://api.netatmo.com/oauth2/token";
const char* URL_GET_STATIONS_DATA = "https://api.netatmo.com/api/getstationsdata";
const char* URL_GET_HOME_DATA = "https://api.netatmo.com/api/gethomedata";

typedef struct {
    char *client_id = CONFIG_NETATMO_CLIENT_ID;
    char *client_secret = CONFIG_NETATMO_CLIENT_SECRET;
    char *access_token = "";
    char *refresh_token = "";
    char *scope = "read_station read_camera access_camera write_camera read_presence access_presence write_presence read_thermostat write_thermostat read_smokedetector";
    uint32_t *expiration = 0;
} auth_struct;

class EspAtmo
{
  public:
    void auth(void);
  private:
    auth_struct latest_auth;
    bool debug_enabled = true;
};
#endif