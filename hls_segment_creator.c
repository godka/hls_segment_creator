#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
typedef struct resolution{
	int width;
	int height;
	unsigned int rate;
	unsigned int maxrate;
} Tresolution;

Tresolution rlist [] = {
	{ 0, 1080, 0, 0 },
	{ 0, 720, 0, 0 },
	{ 0, 480, 0, 0 },
	{ 0, 360, 0, 0 }
};

#define TresolutionCount sizeof(rlist) / sizeof(Tresolution)

int GetFileNameWithoutExtension(char* filename){
	int i;
	int tmp = strlen(filename);
	for (i = tmp - 1; i >= 0; i--){
		if (filename[i] == '.'){
			return i;
		}
	}
	return 0;
}

int GetSystemOutput(char* cmdstring)
{
	char cmd[256] = {0};
	sprintf(cmd, "ffmpeg -i \"%s\" 2> %s.log", cmdstring,cmdstring);
	return system(cmd);
}

int GetFileDetail(const char* filename, int *width, int *height, int *bitrate){
	int tmpwidth = 0, tmpheight = 0, tmpbitrate = 0,t,lockatt = 0;
	char* p;
	char tmp[1024] = { 0 };
	char buffer[1024] = { 0 };
	sprintf(tmp, "%s.log", filename);
	FILE* ofile = fopen(tmp, "r");
	while (!feof(ofile)){
		fgets(tmp, 1024, ofile);
		tmpwidth = 0, tmpheight = 0, tmpbitrate = 0;
		char* p = strtok(tmp, ",");
		while (p != NULL){
			if (!lockatt)
				t = sscanf(p, " %dx%d [%s]", &tmpwidth, &tmpheight, buffer);
			if (t == 3){
				lockatt = 1;
				t = 0;
			}
			else{
				sscanf(p, "%d  kb/s", &tmpbitrate);
			}
			if (tmpwidth > 0 && tmpheight > 0 && tmpbitrate > 0){
				*width = tmpwidth;
				*height = tmpheight;
				*bitrate = tmpbitrate;
				fclose(ofile);
				return 0;
			}
			p = strtok(NULL, ",");
		}
	}
	fclose(ofile);
	return 1;
}

int GetFileName(char* filename){
	int i;
	int tmp = strlen(filename);
	for (i = tmp - 1; i >= 0; i--){
		if (filename[i] == '\\' || filename[i] == '/'){
#ifdef WIN32
			return i + 1;
#else
			return i;
#endif
		}
	}
	return 0;
}

int main(int argc, char* argv[])
{
	char* filename, *strformat;;
	char tmp[1024] = { 0 }, realfilename[255] = { 0 },pfilename[255] = { 0 };
	int realfileindex, pfileindex,i,oriwidth,oriheight,oribitrate,mostchecked;
	FILE* istream; 
#ifdef WIN32
	strformat = "start ffmpeg -y -i %s -pix_fmt yuv420p -vcodec libx264 -acodec aac -strict -2 -r 25 -profile:v baseline  -b:v %dk -maxrate %dk -force_key_frames 50 -s %dx%d -map 0 -flags -global_header -f segment -segment_list %s_%d_%d.m3u8 -segment_time 10 -segment_format mpeg_ts -segment_list_type m3u8 %s_%d_%d_%%05d.ts";
#else
	strformat = "ffmpeg -y -i %s -pix_fmt yuv420p -vcodec libx264 -acodec aac -strict -2 -r 25 -profile:v baseline  -b:v %dk -maxrate %dk -force_key_frames 50 -s %dx%d -map 0 -flags -global_header -f segment -segment_list %s_%d_%d.m3u8 -segment_time 10 -segment_format mpeg_ts -segment_list_type m3u8 %s_%d_%d_%%05d.ts &";
#endif
	if (argc > 1){
		filename = argv[1];
		GetSystemOutput(filename);
		if (GetFileDetail(filename, &oriwidth, &oriheight, &oribitrate) == 0){
			realfileindex = GetFileNameWithoutExtension(filename);
			pfileindex = GetFileName(filename);
			if (pfileindex - realfileindex > 0){
				realfileindex = strlen(filename);
			}
			memcpy(realfilename, filename + pfileindex, realfileindex - pfileindex);
			memcpy(pfilename, filename + pfileindex, strlen(filename) - pfileindex);
			puts(pfilename);
			sprintf(tmp, "%s.m3u8", realfilename);
			istream = fopen(tmp, "w");
			fprintf(istream, "#EXTM3U\n");
			/*auto hold code begin*/
			for (i = 0; i < TresolutionCount; i++){
				//aspect ratio
				rlist[i].width = rlist[i].height * oriwidth / oriheight;
				if (rlist[i].width % 2 == 1){
					rlist[i].width++;
				}
				rlist[i].rate = (unsigned int) oribitrate * (unsigned int) rlist[i].width * (unsigned int) rlist[i].height / ((unsigned int) oriwidth * (unsigned int) oriheight);
				rlist[i].maxrate = (unsigned int) rlist[i].rate *(unsigned int) 15 / (unsigned int) 10;
				sprintf(tmp, strformat,
					pfilename, rlist[i].rate, rlist[i].maxrate,
					rlist[i].width, rlist[i].height,
					realfilename, rlist[i].width, rlist[i].height,
					realfilename, rlist[i].width, rlist[i].height);
				fprintf(istream, "#EXT-X-STREAM-INF:PROGRAM-ID=1,BANDWIDTH=%d000\n", rlist[i].maxrate);
				fprintf(istream, "%s_%d_%d.m3u8\n", realfilename, rlist[i].width, rlist[i].height);
				system(tmp);
			}
			fprintf(istream, "#EXT-X-ENDLIST");
			fclose(istream);
			puts("all tasks done");
		}
		else{
			printf("read file failed,cannot calc width and height.\n");
		}
	}
	else{
		puts("usage:hls_segment_creator filename");
	}
#ifdef WIN32
	getchar();
#endif
	return 0;
}

