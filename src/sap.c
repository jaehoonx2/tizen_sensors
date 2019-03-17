#include "helloaccessory.h"
#include "main_app.h"
#include <sensor.h>
#include <sap.h>
#include <glib.h>
#include <string.h>

#define HELLO_ACC_ASPID "/sample/hello"

extern int hrm_data;
extern float accel_data[ACCLEN];
extern double time_in_mill;

struct priv {
   sap_agent_h agent;
   sap_socket_h socket;
   sap_peer_agent_h peer_agent;
};

static gboolean agent_created = FALSE;

static struct priv priv_data = { 0 };

static void on_service_connection_terminated(sap_peer_agent_h peer_agent,
                    sap_socket_h socket,
                    sap_service_connection_terminated_reason_e result,
                    void *user_data)
{
   switch (result) {
   case SAP_CONNECTION_TERMINATED_REASON_PEER_DISCONNECTED:
      break;

   case SAP_CONNECTION_TERMINATED_REASON_DEVICE_DETACHED:
      break;

   case SAP_CONNECTION_TERMINATED_REASON_UNKNOWN:
      break;
   }

   sap_socket_destroy(priv_data.socket);
   priv_data.socket = NULL;
}

void on_data_received(sap_socket_h socket, unsigned short int channel_id){

	unsigned int payload_length;
 	char * msg = g_strdup_printf("%d %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %.0lf",
 			hrm_data,
			accel_data[0],  accel_data[1],  accel_data[2],
			accel_data[3],  accel_data[4],  accel_data[5],
			accel_data[6],  accel_data[7],  accel_data[8],
			accel_data[9],  accel_data[10], accel_data[11],
			accel_data[12], accel_data[13], accel_data[14],
			accel_data[15], accel_data[16], accel_data[17],
			accel_data[18], accel_data[19], accel_data[20],
			accel_data[21], accel_data[22], accel_data[23],
			accel_data[24], accel_data[25], accel_data[26],
			accel_data[27], accel_data[28], accel_data[29],
			time_in_mill);

 	payload_length = strlen(msg);
 	dlog_print(DLOG_INFO, LOG_TAG, "payload length : %d timestamp : %.0lf", payload_length, time_in_mill);
 	sap_socket_send_data(priv_data.socket, HELLO_ACC_CHANNELID, payload_length, msg);	// send the msg to the Consumer(A)
 	g_free(msg);
}

static void on_service_connection_requested(sap_peer_agent_h peer_agent,
                   sap_socket_h socket,
                   sap_service_connection_result_e result,
                   void *user_data)
{
   priv_data.socket = socket;
   priv_data.peer_agent = peer_agent;

   sap_peer_agent_set_service_connection_terminated_cb
      (priv_data.peer_agent, on_service_connection_terminated, user_data);

   sap_socket_set_data_received_cb(socket, on_data_received, peer_agent);

   sap_peer_agent_accept_service_connection(peer_agent);
}

static void on_agent_initialized(sap_agent_h agent,
             sap_agent_initialized_result_e result,
             void *user_data)
{
   switch (result) {
   case SAP_AGENT_INITIALIZED_RESULT_SUCCESS:

      sap_agent_set_service_connection_requested_cb(agent,
                           on_service_connection_requested,
                           NULL);

      priv_data.agent = agent;
      agent_created = TRUE;
      break;

   case SAP_AGENT_INITIALIZED_RESULT_DUPLICATED:
      break;

   case SAP_AGENT_INITIALIZED_RESULT_INVALID_ARGUMENTS:
      break;

   case SAP_AGENT_INITIALIZED_RESULT_INTERNAL_ERROR:
      break;

   default:
      break;
   }
}

static void on_device_status_changed(sap_device_status_e status, sap_transport_type_e transport_type,
                 void *user_data)
{
   switch (transport_type) {
   case SAP_TRANSPORT_TYPE_BT:
      break;

   case SAP_TRANSPORT_TYPE_BLE:
      break;

   case SAP_TRANSPORT_TYPE_TCP:
      break;

   case SAP_TRANSPORT_TYPE_USB:
      break;

   case SAP_TRANSPORT_TYPE_MOBILE:
      break;

   default:
      break;
   }

   switch (status) {
   case SAP_DEVICE_STATUS_DETACHED:

      if (priv_data.peer_agent) {
         sap_socket_destroy(priv_data.socket);
         priv_data.socket = NULL;
         sap_peer_agent_destroy(priv_data.peer_agent);
         priv_data.peer_agent = NULL;

      }

      break;

   case SAP_DEVICE_STATUS_ATTACHED:
      break;

   default:
      break;
   }
}

gboolean agent_initialize()
{
   int result = 0;

   do {
      result = sap_agent_initialize(priv_data.agent, HELLO_ACC_ASPID, SAP_AGENT_ROLE_PROVIDER,
                     on_agent_initialized, NULL);

   } while (result != SAP_RESULT_SUCCESS);

   return TRUE;
}

void initialize_sap(){
   sap_agent_h agent = NULL;

      sap_agent_create(&agent);

      if (agent == NULL){}
      else{}

      priv_data.agent = agent;

      sap_set_device_status_changed_cb(on_device_status_changed, NULL);

      agent_initialize();
}

