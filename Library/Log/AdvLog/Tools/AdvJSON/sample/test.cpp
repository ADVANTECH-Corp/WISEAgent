#include <stdio.h>
#include <string.h>
#include "AdvJSON.h"

int main(int argc, char **argv) {
	char jsonbuffer[4096] = {0};
	//int result;
	
#if 0
	FILE *fp = fopen("example.json","r");
	int result = fread(jsonbuffer, sizeof(jsonbuffer), 1, fp);
	fclose(fp);
	//printf("jsonbuffer = (%d)\n%s\n", strlen(jsonbuffer),jsonbuffer);
	//int result = JSON_Validator(jsonbuffer);
	//printf("result = %d, len = %d\n", result, (int)strlen(jsonbuffer));
	//JSON_ShowError(jsonbuffer, result);
	
	//result = AdvJSON::Validator(jsonbuffer);
	//ADV_C_DEBUG(COLOR_CYAN,"result = %d, len = %d\n", result, (int)strlen(jsonbuffer));
	
	AdvJSON json(jsonbuffer);
	
	json.Print(0);
	printf("========================================================\n");
	//json.PrintLink(0);
	return 0;
	//AdvJSON::array_t value = {"one", "two", true};
	//json::array_t value = {"one", "two", true};
	//cout << value << "\n";

	//AdvJSON json("{}");
	json.New()["start"] = {"one", "two", {false, "three", {"six", true, "four"}}};
	//json.Print();
	json.New()["start"] += {{1,2,1.1}, {10.55f, 10000.3d}};
	//json.Print();
	json.Edit()["start"] = {"aaa", "bbb", {true, {"ccc"}, {"ddd", false, {"fff"}}}};
	
	//json.New()["start"] = {"AAA"};
	json.Print();
	//AdvJSON node = json.New()["start"];
	//node.Conf();
	
	return 0;
	json.New()["start"][10][{"a","b","c","e"}] = {{1,2,3},"bbb","ccc", 1};
	json.Print();
	
	//json.New()["start"][0] = node;
	//json.New()["start"][0] = "0";
	//json.New()["start"][0]["0"] = "000";
	//json.New()["a"]["b"]["c"]["d"] = "e";
	
	JSONode *node = JSON_Parser("{}");
	JSON_Cmd(node,"New [start][1]", "test", 5);	
	printf("[node]\n");
	JSON_Print(node);
	JSON_PrintLink(node);

	/*JSONode *copy = JSON_Copy(node);
	
	printf("[copy]\n");
	JSON_Print(copy);
	JSON_PrintLink(copy);
	
	
	JSON_Destory(&node);*/
	
	printf("[node]\n");
	JSON_Print(node);
	
	/*printf("[copy]\n");
	JSON_Print(copy);
	JSON_PrintLink(copy);*/
	
	json.New()["start"][0] = node;
	json.Print();
	JSON_Print(json.GetNode());
	//json.Print(0);
	JSON_Destory(&node);
	/*char result[256] = {0};
	JSONode *node = json.GetNode();
	
	JSON_Get(node,"[a][b][c][d]", result, sizeof(result));
	printf("result = %s\n", result);
	
	JSON_Get(node,"[start][0][0]", result, sizeof(result));
	printf("result = %s\n", result);
	
	JSON_Get(node,"/a/b/c/d", result, sizeof(result));
	printf("result = %s\n", result);
	
	JSON_Get(node,"/start/0/0", result, sizeof(result));
	printf("result = %s\n", result);
	
	JSON_Cmd(node,"New [start][1]", "test", 5);	
	JSON_Get(node,"[start][1]", result, sizeof(result));
	printf("result = %s\n", result);
	JSON_Print(node);
	
	
	JSON_Cmd(node,"Edit [start][1]", "test2", 5);	
	JSON_Get(node,"/start/1/", result, sizeof(result));
	printf("result = %s\n", result);
	JSON_Print(node);
	
	
	JSON_Cmd(node,"Delete [start][1]", NULL, 0);
	JSON_Get(node,"[start][1]", result, sizeof(result));
	printf("result = %s\n", result);
	JSON_Print(node);*/
	
	//node += {false, "three", "four", "five"};
	//node.Array() = {"six"};
	/*FILE *fp = fopen("example.json","r");
	result = fread(jsonbuffer, sizeof(jsonbuffer), 1, fp);
	fclose(fp);
	//printf("jsonbuffer = (%d)\n%s\n", strlen(jsonbuffer),jsonbuffer);
	//int result = JSON_Validator(jsonbuffer);
	//printf("result = %d, len = %d\n", result, (int)strlen(jsonbuffer));
	//JSON_ShowError(jsonbuffer, result);
	
	result = AdvJSON::Validator(jsonbuffer);
	//ADV_C_DEBUG(COLOR_CYAN,"result = %d, len = %d\n", result, (int)strlen(jsonbuffer));
	
	AdvJSON json(jsonbuffer);
	string temp = json["IoTGW"]["WSN"]["WSN0"]["Info"]["e"][0][0]["n"].Value();
	//ADV_C_DEBUG(COLOR_CYAN,"temp = %s\n",temp.c_str());
	
	
	node = json["IoTGW"]["WSN"]["WSN0"]["Info"]["e"][0]["n"];
	node = "Fred";
	//node << "name";
	int count = json["IoTGW"]["WSN"]["WSN0"]["Info"]["e"].Size();
	//ADV_C_DEBUG(COLOR_CYAN,"count = %d\n",count);
	
	
	temp = json["IoTGW"]["WSN"]["WSN0"]["Info"]["e"][1]["n"].Value();
	//ADV_C_DEBUG(COLOR_CYAN,"temp = %s\n",temp.c_str());
	
	
	node = json["IoTGW"]["WSN"]["WSN0"]["Info"]["e"];
	temp = node[2]["n"].Value();
	//ADV_C_DEBUG(COLOR_CYAN,"temp = %s\n",temp.c_str());
	
	temp = json["IoTGW"]["ver"].Value();
	//ADV_C_DEBUG(COLOR_CYAN,"temp = %s\n",temp.c_str());
	
	node = json["IoTGW"]["WSN"]["WSN0"];
	//node << "aaa";
	node = json["IoTGW"]["WSN"];
	json.Print();
	return 0;*/
	
	//AdvJSON json(jsonbuffer);
	
	//json.Print();
	/*printf("true = 		%d\n", json["IoTGW"] == NULL);
	printf("false = 	%d\n", json["Info"] == NULL);
	printf("compare1 = 	%d\n", json["Info"] == json["IoTGW"]);
	printf("compare2 = 	%d\n", json["Info"] == json["Info"]);
	printf("\n");
	
	string temp = json["Info"]["e"][0]["n"].Value();
	printf("temp = %s\n",temp.c_str());
	json["Info"]["e"][0]["n"] = "Sen";*/
	
	printf("========================================================\n");
#else 


	//json.New()["root"] = {{"1","2","3"}};
	//C[{"n"}];
	
	//JSON_Print(C[{"n"}]({"SenHubList"}));
	
	/*json.New()["IoTGW"]["WSN"]["WSN0"]["Info"][{"e","bn"}] = {
																{
																 C[{"n","sv","asm"}]({"SenHubList","0000000EC6F0F830,0000000EC6F0F831","r"}),
																 C[{"n","sv","asm"}]({"Neighbor","0000000EC6F0F830","r"}),
																 C[{"n","v","asm"}]({"Health",80,"r"}),
																 C[{"n","sv","asm"}]({"Name","WSN0","r"}),
																 C[{"n","sv","asm"}]({"sw","1.0.0.1","r"}),
																 C[{"n","bv","asm"}]({"reset",0,"rw"})
																},
																"Info"
															  };*/
	/*AdvJSON iot("{}");
	{
		AdvJSONCreator D(iot);													  
		iot.New()["IoTGW"]["WSN"]["WSN0"][{"Info","bn","ver"}] = {
			D[{"e","bn"}]({
				{
					C[{"n","sv","asm"}]({"SenHubList","0000000EC6F0F830,0000000EC6F0F831","r"}),
					C[{"n","sv","asm"}]({"Neighbor","0000000EC6F0F830","r"}),
					C[{"n","v","asm"}]({"Health",80,"r"}),
					C[{"n","sv","asm"}]({"Name","WSN0","r"}),
					C[{"n","sv","asm"}]({"sw","1.0.0.1","r"}),
					C[{"n","bv","asm"}]({"reset",0,"rw"}),
				},
				"Info"
			}),
			"0000852CF4B7B0E8",
			1
		};
	}
	iot.Print(0);*/
	
	
	AdvJSON root("{}");
	AdvJSONCreator C(root);								
	root.New()["IoTGW"]["WSN"]["WSN0"][{"Info","bn","ver"}] = {
		C[{"e","bn"}]({
			{
				C[{"n","sv","asm"}]({"SenHubList","0000000EC6F0F830,0000000EC6F0F831","r"}),
				C[{"n","sv","asm"}]({"Neighbor","0000000EC6F0F830","r"}),
				C[{"n","v","asm"}]({"Health",80,"r"}),
				C[{"n","sv","asm"}]({"Name","WSN0","r"}),
				C[{"n","sv","asm"}]({"sw","1.0.0.1","r"}),
				C[{"n","bv","asm"}]({"reset",0,"rw"}),
			},
			"Info"
		}),
		"0000852CF4B7B0E8",
		1
	};
	root.Edit()["IoTGW"]["WSN"]["WSN0"][{"1","2","3"}] = {{1,1},{2,2},{3,3}};
	
	
	root.Print(0);
	printf("========================================================\n");
	root.PrintLink(0);
	/*json.Print();
	printf("\n");
	json.Print(0);
	
	char *fmt = "{ 																	\
				  \"IoTGW\": {														\
						\"WSN\": {													\
							\"WSN0\": {												\
								\"Info\": {											\
									\"e\": [{\"n\":\"%s\",\"sv\":\"%s\",\"asm\":\"%s\"},			\
											{\"n\":\"%s\",\"sv\":\"%s\",\"asm\":\"%s\"},			\
											{\"n\":\"%s\",\"v\":%d,\"asm\":\"%s\"},					\
											{\"n\":\"%s\",\"sv\":\"%s\",\"asm\":\"%s\"},			\
											{\"n\":\"%s\",\"sv\":\"%s\",\"asm\":\"%s\"},			\
											{\"n\":\"%s\",\"bv\":%d,\"asm\":\"%s\"}],				\
									\"bn\":\"Info\"													\
									},																\
							  \"bn\":\"%s\",														\
							  \"ver\":%d															\
							}";
	
	
	char result[1024];
	sprintf(result,fmt,
	"SenHubList","0000000EC6F0F830,0000000EC6F0F831","r",
	"Neighbor","0000000EC6F0F830","r",
	"Health",80,"r",
	"Name","WSN0","r",
	"sw","1.0.0.1","r",
	"reset",0,"rw",
	"0000852CF4B7B0E8",
	1
	);
	
	
	printf("result = \n%s\n", result);
	
	
	cJSON *root,*fmt, *array;
	root=cJSON_CreateObject();	
	cJSON_AddItemToObject(root, "name", cJSON_CreateString("Jurassic World"));
	cJSON_AddItemToObject(root, "format", fmt=cJSON_CreateObject());
	cJSON_AddStringToObject(fmt,"type",		"rect");
	cJSON_AddNumberToObject(fmt,"width",		1920);
	cJSON_AddNumberToObject(fmt,"height",		1080);
	cJSON_AddFalseToObject (fmt,"interlace");
	cJSON_AddItemToObject(fmt,"frame rate", array=cJSON_CreateArray());
	cJSON_AddItemToArray(array,cJSON_CreateNumber(15));
	cJSON_AddItemToArray(array,cJSON_CreateNumber(24));
	cJSON_AddItemToArray(array,cJSON_CreateNumber(30));
	
	
	
	cJSON *root = cJSON_Parse(my_json_string);
	char *name = cJSON_GetObjectItem(root,"name")->valuestring;
	
	cJSON *format = cJSON_GetObjectItem(root,"format");
	char *type = cJSON_GetObjectItem(format,"type")->valuestring;
	int width = cJSON_GetObjectItem(format,"width")->valueint;
	int height = cJSON_GetObjectItem(format,"height")->valueint;
	int interlace = cJSON_GetObjectItem(format,"interlace")->valueint;
	
	cJSON *array = cJSON_GetObjectItem(format, "frame rate");
	int framerate0 = cJSON_GetArrayItem(array,0)->valueint;
	int framerate1 = cJSON_GetArrayItem(array,1)->valueint;
	int framerate2 = cJSON_GetArrayItem(array,2)->valueint;
	
	
	Json::Value movie;
	Json::Value array;
	array.append(15);
	array.append(24);
	array.append(30);
	movie["name"] = "Jurassic World";
	movie["format"]["type"] = "rect";
	movie["format"]["width"] = 1920;
	movie["format"]["height"] = 1080;
	movie["format"]["interlace"] = false;
	movie["format"]["frame rate"] = array;
	
	
	Json::Reader reader;
	Json::Value movie;
	reader.parse(my_json_string, movie);
	char *name = movie["name"].asCString();
	char *type = movie["format"]["type"].asCString();
	int width = movie["format"]["width"].asInt();
	int height = movie["format"]["height"].asInt();
	bool interlace = movie["format"]["interlace"].asBool();
	int framerate0 = movie["format"]["frame rate"][0].asInt();
	int framerate1 = movie["format"]["frame rate"][1].asInt();
	int framerate2 = movie["format"]["frame rate"][2].asInt();
	
	
	
	
	
	Json::Reader reader;
	Json::Value movie;
	reader.parse(my_json_string, movie);
	char *name = movie["name"].asCString();
	char *type = movie["format"]["type"].asCString();
	int width = movie["format"]["width"].asInt();
	int height = movie["format"]["height"].asInt();
	bool interlace = movie["format"]["interlace"].asBool();
	int framerate0 = movie["format"]["frame rate"][0].asInt();
	int framerate1 = movie["format"]["frame rate"][1].asInt();
	int framerate2 = movie["format"]["frame rate"][2].asInt();
	
	
	rapidjson::Document movie;
	movie.Parse<0>(my_json_string);
	char *name = movie["name"].GetString();
	char *type = movie["format"]["type"].GetString();
	int width = movie["format"]["width"].GetInt();
	int height = movie["format"]["height"].GetInt();
	bool interlace = movie["format"]["interlace"].GetBool();
	int framerate0 = movie["format"]["frame rate"][0].GetInt();
	int framerate1 = movie["format"]["frame rate"][1].GetInt();
	int framerate2 = movie["format"]["frame rate"][2].GetInt();
	
	
	
	
	rapidjson::Document movie(rapidjson::kObjectType);
	rapidjson::Value array(rapidjson::kArrayType);
	rapidjson::Document::AllocatorType& allocator = movie.GetAllocator();
	array.PushBack(15, allocator).PushBack(24, allocator).PushBack(30, allocator);
	movie.AddMember("name", "Jurassic World", allocator);
	rapidjson::Value format(rapidjson::kObjectType);
	format.AddMember("type", "rect", allocator);
	format.AddMember("width", 1920, allocator);
	format.AddMember("height", 1080, allocator);
	format.AddMember("interlace", false, allocator);
	format.AddMember("frame rate", array, allocator);
	movie.AddMember("format", format, allocator);*/
	
	
	
	
	//AdvJSON movie("{}");
	//movie.New()["name"] = "Jurassic World";
	//movie.New()["format"]["type"] = "rect";
	//movie.New()[{"type", "mode"}] = {{0,1},{2,3}};
	//movie.New()["format"][{"width", "height"}] = {1920, 1080};
	/*movie.New()["format"][{"type", "width", "height", "interlace", "frame rate"}] 
		= {"rect" , 1920, 1080, false, {15,24,30}};*/
	
	/*movie.Edit()["format"]["frame rate"] = {8,16,32};
	movie.New()["format"]["frame rate"] += {64,128,256};


	AdvJSON node = movie["format"];
	node.Edit()[{"type", "frame rate","resolution"}] 
		     = { "edit",      {1,2,3}, "1920x1080"};


	node = movie["format"];
	node.New()[{"type", "frame rate","resolution"}] 
		     = { "new",      {4,5,6}, "1920x1080"};*/
	
	//movie.Print(0);
	
	
	
	/*AdvJSONCreator C(movie);
	JSON_Print(C[{"Video" , "Audio" }]({ {"H.264", "6000"} , {"AAC","14400"} }));
	
	
	
	
	movie.New()["info"] = C[{"Video" , "Audio" }]({ {"H.264", "6000"} , {"AAC","14400"} });
	
	
	
	
	movie.Print(0);*/
	
	/*AdvJSON movie(my_json_string);
	char *name = movie["name"].String().c_str();
	char *type = movie["format"]["type"].String().c_str();
	int width = movie["format"]["width"].Int();
	int height = movie["format"]["height"].Int();
	bool interlace = movie["format"]["interlace"].Bool();
	int framerate0 = movie["format"]["frame rate"][0].Int();
	int framerate1 = movie["format"]["frame rate"][1].Int();
	int framerate2 = movie["format"]["frame rate"][2].Int();*/
	
	
#endif	
	//json.New()["vendor"]["company"] = "Advantech";
	//json.New()["vendor"]["author"] = "Fred";
	//json.New()["vendor"]["location"] = "LinKou";
	
	//json.New()["vendor"][0]["company"] = "Advantech";
	//json.New()["vendor"][0]["author"] = {"Fred", "Eric", "William"};
	//json.New()["vendor"][0]["location"] = "LinKou";
	//json.New()["vendor"][0]["number"] = {1.1f,100,-10};
	
	/*json.New()["vendor"][2]["company"] = "Advansus";
	json.New()["vendor"][2]["author"] = "Fred";
	json.New()["vendor"][2]["location"] = "LinKou";*/
	
	//json.New()["vendor"][1]["location"]("L");
	
	//json["vendor"][1]["location"].Delete();
	//json["vendor"][0]["company"].Delete();
	
	
	/*json.New()["a"]["b"]["c"]["d"] = "e";
	json["a"]["b"]["c"]["d"].Delete();
	json["a"]["b"]["c"].Delete();
	json["a"]["b"].Delete();
	json["a"].Delete();*/
	//AdvJSON json1("{}");
	printf("========================================================\n");
	/*json1.New()["c"]["d"]["e"] = "0";
	json1.New()["c"]["d"]["f"] = "1";
	json1["c"]["d"]["e"].Delete();
	json1["c"]["d"]["f"].Delete();
	json1.New()["c"]["d"]["f"] = "2";
	json1["c"]["d"].Delete();
	json1.New()["c"]["d"]["f"] = "3";*/
	
	/*json1.New()["a"][10]["i"] = "iii";
	json1.New()["a"][10]["j"] = "jjj";
	json1.New()["a"][10]["k"] = "kkk";
	json1.New()["a"][10]["l"]["x"] = "xxx";
	json1.New()["a"][10]["l"]["y"] = "yyy";
	json1.New()["a"][10]["l"]["z"] = "zzz";
	json1.New()["a"][5][0] = "5";
	json1.New()["a"][5][1] = "55";
	json1.New()["a"][5][2] = "555";
	json1.Edit()["a"][10]["l"]["y"].Erase();
	json1.Edit()["a"][10]["l"]["x"].Erase();
	json1.Edit()["a"][10]["l"]["z"].Erase();
	json1.Print(0);
	//json1.Edit()["a"][10].Erase();
	//json1.Edit()["a"][5][0].Erase();
	json1.Edit()["a"][10]["i"].Erase();
	json1.Edit()["a"] << "name";*/
	
	//json1.Edit()["a"] = {"0", 0, json1};
	
	/*json1.New()["a"][10]["i"] = "iii";
	json1.New()["a"][10]["j"] = "jjj";
	json1.New()["a"][10]["k"] = "kkk";
	json1.New()["a"][10]["l"]["x"] = "xxx";
	json1.New()["a"][10]["l"]["y"] = "yyy";
	json1.New()["a"][10]["l"]["z"] = "zzz";*/
	printf("========================================================\n");
	//json1.New()["c"][0]["d"] = {"0", "1", "2"};
	//json1.New()["c"][0]["d"][0];
	//json1["c"][0]["d"][0].Delete();
	//json1.New()["c"][0]["d"] = {"0", "1", "2"};
	//printf("========================================================\n");
	//json1.New()["c"][0]["d"][0] = "10";
	//printf("========================================================\n");
	//json1.Print(0);
	/*printf("D=S=======================================================\n");
	json1["c"][0]["d"][0].Delete();
	printf("D=E=======================================================\n");
	json1.Print(0);*/
	
	/*AdvJSON json2("{}");
	json2.New()["c"][0]["d"][0] = "0";
	json2.Print(0);
	printf("D=S=======================================================\n");
	json2["c"][0]["d"].Delete();
	printf("D=E=======================================================\n");*/
	/*AdvJSON json3("{}");
	json3.New()["a"][0]["b"][0]["c"][0]["d"][0] = {"0"};
	json3["a"][0]["b"][0]["c"][0].Delete();
	
	AdvJSON json4("{}");
	json4.New()["a"][0]["b"][0]["c"][0]["d"][0] = {"0"};
	json4["a"][0]["b"][0]["c"].Delete();
	
	AdvJSON json5("{}");
	json5.New()["a"][0]["b"][0]["c"][0]["d"][0] = {"0"};
	json5["a"][0]["b"][0].Delete();
	
	AdvJSON json6("{}");
	json6.New()["a"][0]["b"][0]["c"][0]["d"][0] = {"0"};
	json6["a"][0]["b"].Delete();
	json6.New()["a"][0]["s"] = "s";
	
	AdvJSON json7("{}");
	json7.New()["a"][0]["b"][0]["c"][0]["d"][0] = {"0"};
	json7["a"][0].Delete();
	
	AdvJSON json8("{}");
	json8.New()["a"][0]["b"][0]["c"][0]["d"][0] = {"0"};
	json8["a"].Delete();*/
	
	//json1.Print();
	//json2.Print();
	/*json3.Print();
	json4.Print();
	json5.Print();
	json6.Print();
	json7.Print();
	json8.Print();*/
	
	
	/*json.Print();
	json.Print(jsonbuffer,4096);
	
	result = AdvJSON::Validator(jsonbuffer);
	printf("result = %d, len = %d\n", result, (int)strlen(jsonbuffer));*/
	return 0;

}
