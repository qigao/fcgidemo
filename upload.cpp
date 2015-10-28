#include <stdlib.h>
#include <string.h>
#include <alloca.h>
#include <fcgiapp.h>
#include <syslog.h>
#include "Parser.h"

#define printf(...) FCGX_FPrintF(request->out, __VA_ARGS__)
#define get_param(KEY) FCGX_GetParam(KEY, request->envp)

void handle_request(FCGX_Request *request) {
  char *value;

  printf("Content-Type: text/plain\n\n");
  
  const char* content_type = get_param("CONTENT_TYPE");
  const char* content_len = get_param("CONTENT_LENGTH");
  int content_length = strtol(content_len,NULL,10);                                                               
  int max_temp_buf_size =(int)(1024*1024*0.1);
  int temp_buf_size = content_length < max_temp_buf_size ? content_length:max_temp_buf_size;
  char* temp_buf = new char[temp_buf_size+1];

  const char* multipart_form_data = "multipart/form-data;";                                                         
  int multipart_form_data_len = (int)strlen(multipart_form_data);                                                   
  MPFD::Parser *POSTParser = new MPFD::Parser();
  POSTParser->SetContentType(content_type);
  POSTParser->SetUploadedFilesStorage(MPFD::Parser::StoreUploadedFilesInFilesystem);
  POSTParser->SetTempDirForFileUpload("/tmp");
  for (int cur_len; (cur_len = FCGX_GetStr(temp_buf,temp_buf_size,request->in))>0;)
    {
      POSTParser->AcceptSomeData(temp_buf,cur_len);
    }
  delete[] temp_buf;
  temp_buf = NULL;
  
  std::map<std::string,MPFD::Field *> fields=POSTParser->GetFieldsMap();
  std::map<std::string,MPFD::Field *>::iterator it;                       
  for (it=fields.begin(); it != fields.end(); it++)
    {                         
      MPFD::Field* field = fields[it->first];
      if (fields[it->first]->GetType()==MPFD::Field::TextType)
        {
          printf("field_type:%s",fields[it->first]->GetTextTypeContent().c_str());
        }
      else
        {
          printf("file_name:%s",fields[it->first]->GetFileName().c_str());
          printf("\n");
          printf("temp_file_name:%s",fields[it->first]->GetTempFileName().c_str());
        }
    }
  }

int main(void) {

  FCGX_Request request;

  FCGX_Init(); 
  FCGX_InitRequest(&request, 0, 0);

  while (FCGX_Accept_r(&request) >= 0) {
    handle_request(&request);
    FCGX_Finish_r(&request);
  }

  return EXIT_SUCCESS;
}
