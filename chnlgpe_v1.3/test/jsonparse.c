#include<stdio.h>
#include"cJSON.h"
cJSON* GetJsonObject(char* fileName)
{
    long len;
    char* pContent;
    int tmp;
    cJSON* json;
    FILE* fp = fopen(fileName, "rb+");
    json = cJSON_CreateObject();
    if(!fp)
    {
        return NULL;
    }
    fseek(fp,0,SEEK_END);
    len=ftell(fp);
    if(0 == len)
    {
    	printf("no length\n");
        return NULL;
    }
    printf("%d\n",len);
    fseek(fp,0,SEEK_SET);
    pContent = (char*) malloc (sizeof(char)*len);
    tmp = fread(pContent,1,len,fp);
    fclose(fp);
    json=cJSON_Parse(pContent);
    if (!json)
    {
        return NULL;
    }
    free(pContent);
    return json;
}
int main (int argc, char ** argv)
{
	char *str;
	cJSON* json = GetJsonObject("svg_runningdata_config.json");
	
	if (json)
	{
		str = cJSON_GetObjectItem(cJSON_GetArrayItem(json,0),"packetID")->valuestring;
		//str = cJSON_Print(json);
		printf("%s\n",str);
		cJSON_Delete(json);
	}
	return 0;
}
#if 0
root = cJSON_Parse(out);
	char *str = cJSON_GetObjectItem(root,"message")->valuestring;
	int count = cJSON_GetObjectItem(root,"allRowCount")->valueint;
	arr =  cJSON_GetObjectItem(root,"array")
	
	for(i=0;i<count;i++)
	{
		cJSON *fmt = cJSON_GetArrayItem(arr,i);
		char *value = cJSON_GetObjectItem(fmt,"value")->valuestring;
		char *id = cJSON_GetObjectItem(fmt,"id")->valuestring;
	}
	str = cJSON_GetObjectItem(root,"success")->valuestring;
#endif
