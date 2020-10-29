/*
 *  Copyright (C) 2020 Palo Alto Networks Intellectual Property. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


extern "C" {
#include "opof.h"
#ifdef DEBUG
#include "opof_test_util.h"
#endif
}
#include "opof_util.h"
#include "opof_grpc.h"
#include "opof_session_client.h"


/**  \ingroup clientlibrary
* \brief
*
* \param size
* \param sessionRequest_t
* \param addSeesionResponse_t
*
*/
Status SessionTableClient::addSessionClient(int size, sessionRequest_t **s, addSessionResponse_t *resp){

sessionRequest_t *request_c;
sessionRequest request;
addSessionResponse response;
ClientContext context;
Status status;
std::unique_ptr<ClientWriter <sessionRequest> > writer(
        stub_->addSession(&context, &response));

for (int i=0; i< size; i++){
  request_c = s[i];
#ifdef DEBUG
  display_session_request(request_c, "addSessionClient");
#endif
  convertSessionRequest2cpp(request_c, &request);
  writer->Write(request);
  //free(request_c);
}

//free(s);
writer->WritesDone();
status = writer->Finish();
convertAddSessionResponse2c(resp,&response);

return status;
}
/**  \ingroup clientlibrary
* \brief getSessionClient
*
* \param size
* \param sessionRequest_t
* \param addSeesionResponse_t
*
*/
std::string SessionTableClient::getSessionClient(int sessionid,sessionResponse_t *resp){

  sessionId sid;
  sessionResponse response;
  sid.set_sessionid(sessionid);
  ClientContext context;

  Status status = stub_->getSession(&context, sid, &response);
  convertSessionResponse2c(&response, resp);
  if (status.ok()) {
    return "Success";
  } else {
    std::cout << "RPC getSession: " << sessionid << " failed with error code: " << status.error_code() << ": " << status.error_message()
              << std::endl;
    return "RPC failed";
  }
  //TODO: print timestamps
  //std::cout << "session starttime is: " << response.starttime() << std::endl;
  //std::cout << "session endtime is: " << response.endtime() << std::endl;
}

/**  \ingroup clientlibrary
* \brief deleteSessionClient
*
* \param size
* \param sessionRequest_t
* \param addSeesionResponse_t
*
*/
std::string SessionTableClient::deleteSessionClient(int sessionid,sessionResponse_t *resp){

sessionId sid;
sessionResponse response;
sid.set_sessionid(sessionid);
ClientContext context;
Status status = stub_->deleteSession(&context, sid, &response);

convertSessionResponse2c(&response, resp);
#ifdef DEBUG
  display_session_response(resp, "delSessionClient");
#endif

if (status.ok()) {
  return "Success";
} else {
    std::cout << "RPC Delete Session: " << sessionid << " Failed: " << status.error_code() << ": " << status.error_message()
              << std::endl;
    return "RPC failed";
  }

}
/**  \ingroup clientlibrary
* \brief getClosedSessions
*
* \param size
* \param sessionRequest_t
* \param addSeesionResponse_t
*
*/
unsigned long SessionTableClient::getClosedSessions(statisticsRequestArgs_t *args, sessionResponse_t responses[] ){
  unsigned long sessionCount = 0;
  sessionResponse response;
  statisticsRequestArgs request;
  ClientContext context;
  request.set_pagesize(args->pageSize);
  
  std::unique_ptr<ClientReader <sessionResponse> > reader(
        stub_->getClosedSessions(&context, request));
  while (reader->Read(&response)) {
    if (response.requeststatus() == REQUEST_STATUS::_REJECTED_NO_CLOSED_SESSIONS){
      return 0;
    }
    convertSessionResponse2c(&response, &responses[sessionCount]);
    sessionCount++;
  }
  Status status = reader->Finish();
  if (status.ok()) {
    return sessionCount;
  }
    //std::cout << "getClosedSessions rpc succeeded sesssion count " << sessionCount << std::endl;
  if (status.error_code() == grpc::StatusCode::NOT_FOUND ){
    std::cout << "Get all sessions complete" << std::endl;
    return 0;
  }
  return 0;
}
/**  \ingroup clientlibrary
* \brief getAllSessions
*
* \param size
* \param sessionRequest_t
* \param addSeesionResponse_t
*
*/
Status SessionTableClient::getAllSessions(int pageSize, uint64_t *session_start_id, uint64_t *session_count, sessionResponse_t responses[]){
  unsigned long sessionCount=0;
  Status status;
  sessionResponseArray response;
  statisticsRequestArgs request;
  ClientContext context;
  sessionResponse_t responsec;
  int array_size;

  
  request.set_pagesize(pageSize);
  request.set_startsession(*session_start_id);
  
  status = stub_->getAllSessions(&context, request, &response);
  array_size = response.responsearray_size();
  *session_start_id = response.nextkey();
 

  for (int i = 0; i < array_size; i++ ){
    convertSessionResponse2c(response.mutable_responsearray(i), &responses[i]);
  }

  *session_count = array_size;
  
  return status;
}